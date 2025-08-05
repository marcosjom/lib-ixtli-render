//
//  ScnMemBlock.c
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/core/ScnMemBlock.h"
#include "ixrender/core/ScnArraySorted.h"

//STScnMemBlockPtr

#define STScnMemBlockPtr_Zero { NULL, 0 }

typedef struct STScnMemBlockPtr {
    void*       ptr;  //pointer returned by 'ScnMemBlock_malloc'
    ScnUI32     sz;   //size at 'ScnMemBlock_malloc' call
} STScnMemBlockPtr;

ScnBOOL ScnCompare_ScnMemBlockPtr(const ENScnCompareMode mode, const void* data1, const void* data2, const ScnUI32 dataSz){
    SCN_ASSERT(dataSz == sizeof(STScnMemBlockPtr))
    if(dataSz == sizeof(STScnMemBlockPtr)){
        const STScnMemBlockPtr* d1 = (STScnMemBlockPtr*)data1;
        const STScnMemBlockPtr* d2 = (STScnMemBlockPtr*)data2;
        switch (mode) {
            case ENScnCompareMode_Equal: return d1->ptr == d2->ptr;
            case ENScnCompareMode_Lower: return d1->ptr < d2->ptr;
            case ENScnCompareMode_LowerOrEqual: return d1->ptr <= d2->ptr;
            case ENScnCompareMode_Greater: return d1->ptr > d2->ptr;
            case ENScnCompareMode_GreaterOrEqual: return d1->ptr >= d2->ptr;
            default: SCN_ASSERT(ScnFALSE) break;
        }
    }
    return ScnFALSE;
}

//STScnMemBlockGap

#define STScnMemBlockGap_Zero { 0, 0 }

typedef struct STScnMemBlockGap {
    ScnUI32     iStart;  //start of gap in bytes
    ScnUI32     sz;      //size of gap in bytes
} STScnMemBlockGap;

ScnBOOL ScnCompare_ScnMemBlockGap(const ENScnCompareMode mode, const void* data1, const void* data2, const ScnUI32 dataSz){
    SCN_ASSERT(dataSz == sizeof(STScnMemBlockGap))
    if(dataSz == sizeof(STScnMemBlockGap)){
        const STScnMemBlockGap* d1 = (STScnMemBlockGap*)data1;
        const STScnMemBlockGap* d2 = (STScnMemBlockGap*)data2;
        switch (mode) {
            case ENScnCompareMode_Equal: return d1->iStart == d2->iStart;
            case ENScnCompareMode_Lower: return d1->iStart < d2->iStart;
            case ENScnCompareMode_LowerOrEqual: return d1->iStart <= d2->iStart;
            case ENScnCompareMode_Greater: return d1->iStart > d2->iStart;
            case ENScnCompareMode_GreaterOrEqual: return d1->iStart >= d2->iStart;
            default: SCN_ASSERT(ScnFALSE) break;
        }
    }
    return ScnFALSE;
}

//STScnMemBlockChunk

#define STScnMemBlockChunk_Zero   { NULL, 0, 0, 0 }

typedef struct STScnMemBlockChunk {
    ScnBYTE*    ptr;        //allocated memory
    ScnUI32     ptrSz;      //allocated memory size
    ScnUI32     rngStart;   //first usable position
    ScnUI32     rngAfterEnd;//first non-usable position.
} STScnMemBlockChunk;

//STScnMemBlockState

#define STScnMemBlockState_Zero   { 0, 0 }

typedef struct STScnMemBlockState {
    ScnUI32     szAvail;                //for 'ScnMemBlock_malloc' early return
    ScnUI32     szSmallestMallocFailed; //for 'ScnMemBlock_malloc' early return
} STScnMemBlockState;

//STScnMemBlockOpq

typedef struct STScnMemBlockOpq {
    ScnContextRef       ctx;
    ScnMutexRef         mutex;
    STScnMemBlockChunk  chunk;
    STScnMemBlockCfg    cfg;
    STScnMemBlockState  state;
    ScnArraySortedStruct(ptrs, STScnMemBlockPtr); //returned by 'ScnMemBlock_malloc'
    ScnArraySortedStruct(gaps, STScnMemBlockGap);
} STScnMemBlockOpq;

//

ScnSI32 ScnMemBlock_getOpqSz(void){
    return (ScnSI32)sizeof(STScnMemBlockOpq);
}

void ScnMemBlock_initZeroedOpq(ScnContextRef ctx, void* obj) {
    STScnMemBlockOpq* opq = (STScnMemBlockOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex = ScnContext_allocMutex(opq->ctx);
    //
    ScnArraySorted_init(ctx, &opq->ptrs, 0, 256, STScnMemBlockPtr, ScnCompare_ScnMemBlockPtr);
    ScnArraySorted_init(ctx, &opq->gaps, 0, 128, STScnMemBlockGap, ScnCompare_ScnMemBlockGap);
}

void ScnMemBlock_destroyOpq(void* obj){
    STScnMemBlockOpq* opq = (STScnMemBlockOpq*)obj;
    //
    {
        if(opq->chunk.ptr != NULL){
            ScnContext_mfree(opq->ctx, opq->chunk.ptr);
            opq->chunk.ptr    = NULL;
        }
        ScnMemory_setZeroSt(opq->chunk, STScnMemBlockChunk);
    }
    {
        opq->cfg = (STScnMemBlockCfg)STScnMemBlockCfg_Zero;
        //ScnStruct_stRelease(ScnMemBlockCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
    }
    //
    ScnArraySorted_destroy(opq->ctx, &opq->ptrs);
    ScnArraySorted_destroy(opq->ctx, &opq->gaps);
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNull(&opq->ctx);
}

//

ScnBOOL ScnMemBlock_validateIndexLockepOpq_(STScnMemBlockOpq* opq);

//

ScnBOOL ScnMemBlock_prepare(ScnMemBlockRef ref, const STScnMemBlockCfg* cfg, STScnAbsPtr* dstPtrAfterEnd){
    ScnBOOL r = ScnFALSE;
    STScnMemBlockOpq* opq = (STScnMemBlockOpq*)ScnSharedPtr_getOpq(ref.ptr);
    //
    ScnMutex_lock(opq->mutex);
    if(cfg != NULL && opq->chunk.ptr == NULL){
        const ScnUI32 idxsAlign = (cfg->idxsAlign > 0 ? cfg->idxsAlign : 4);   //individual pointers alignment
        const ScnUI32 sizeAlign = (cfg->sizeAlign > 0 ? cfg->sizeAlign : idxsAlign > 0 ? idxsAlign : 4);   //whole memory block size alignment
        STScnMemBlockChunk chunkN = STScnMemBlockChunk_Zero;
        chunkN.rngStart     = (cfg->idxZeroIsValid ? 0 : idxsAlign);
        chunkN.rngAfterEnd  = (cfg->size + idxsAlign - 1) / idxsAlign * idxsAlign;
        chunkN.ptrSz        = (chunkN.rngAfterEnd + sizeAlign - 1) / sizeAlign * sizeAlign;
        chunkN.ptr          = (ScnBYTE*)ScnContext_malloc(opq->ctx, chunkN.ptrSz, "ScnMemBlock_prepare::chunkN.ptr");
        if(chunkN.ptr != NULL){
            //copy chunk
            {
                if(opq->chunk.ptr != NULL){
                    ScnContext_mfree(opq->ctx, opq->chunk.ptr);
                    opq->chunk.ptr = NULL;
                }
                opq->chunk = chunkN;
            }
            //copy cfg
            {
                opq->cfg            = *cfg;
                //ScnStruct_stRelease(ScnMemBlockCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
                //ScnStruct_stClone(ScnMemBlockCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
                opq->cfg.sizeAlign  = sizeAlign;
                opq->cfg.idxsAlign  = idxsAlign;
                opq->cfg.size       = chunkN.rngAfterEnd;
            }
            //set initial state
            {
                ScnArraySorted_empty(&opq->ptrs);
                ScnArraySorted_empty(&opq->gaps);
                //reset state
                ScnMemory_setZeroSt(opq->state, STScnMemBlockState);
                //register the whole range as the initial gap
                if(chunkN.rngStart < chunkN.rngAfterEnd){
                    STScnMemBlockGap gap = STScnMemBlockGap_Zero;
                    gap.iStart  = chunkN.rngStart;
                    gap.sz      = chunkN.rngAfterEnd - chunkN.rngStart;
                    ScnArraySorted_addPtr(opq->ctx, &opq->gaps, &gap, STScnMemBlockGap);
                    opq->state.szAvail = gap.sz;
                }
            }
#           ifdef SCN_ASSERTS_ACTIVATED
            //TMP
            //ScnMemBlock_validateIndexLockepOpq_(opq);
#           endif
            if(dstPtrAfterEnd != NULL){
                dstPtrAfterEnd->idx = chunkN.rngAfterEnd;
                dstPtrAfterEnd->ptr = chunkN.ptr + chunkN.rngAfterEnd;
            }
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnMemBlock_hasPtrs(ScnMemBlockRef ref){ //allocations made?
    STScnMemBlockOpq* opq = (STScnMemBlockOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq->ptrs.use > 0);
}

ScnUI32 ScnMemBlock_getAddressableSize(ScnMemBlockRef ref){
    STScnMemBlockOpq* opq = (STScnMemBlockOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->cfg.size;
}

STScnAbsPtr ScnMemBlock_getStarAddress(ScnMemBlockRef ref){ //includes the address zero
    STScnMemBlockOpq* opq = (STScnMemBlockOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (STScnAbsPtr){ opq->chunk.ptr, 0 };
}

STScnRangeU ScnMemBlock_getUsedAddressesRng(ScnMemBlockRef ref){ //highest allocated address index
    STScnRangeU r = STScnRangeU_Zero;
    STScnMemBlockOpq* opq = (STScnMemBlockOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->ptrs.use > 0){
        const STScnMemBlockPtr* ptrFirst = &opq->ptrs.arr[0];
        const STScnMemBlockPtr* ptrLast = &opq->ptrs.arr[opq->ptrs.use - 1];
        r.start = (ScnUI32)((ScnBYTE*)ptrFirst->ptr - opq->chunk.ptr);
        r.size  = (ScnUI32)((ScnBYTE*)ptrLast->ptr - opq->chunk.ptr) + ptrLast->sz - r.start;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//allocations

STScnAbsPtr ScnMemBlock_malloc(ScnMemBlockRef ref, const ScnUI32 usableSz){
    STScnAbsPtr r = STScnAbsPtr_Zero;
    STScnMemBlockOpq* opq = (STScnMemBlockOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->cfg.idxsAlign > 0){
        const ScnUI32 sz = (usableSz + opq->cfg.idxsAlign - 1) / opq->cfg.idxsAlign * opq->cfg.idxsAlign;
        if(sz <= opq->state.szAvail && (opq->state.szSmallestMallocFailed == 0 || sz < opq->state.szSmallestMallocFailed)){
            //search for a gap
            STScnMemBlockGap* gStart = opq->gaps.arr;
            STScnMemBlockGap* g = gStart;
            const STScnMemBlockGap* gAfterEnd = g + opq->gaps.use;
            while(g < gAfterEnd){
                if(g->sz >= sz){
                    //register pointer
                    STScnMemBlockPtr ptr = STScnMemBlockPtr_Zero;
                    ptr.ptr = &opq->chunk.ptr[g->iStart];
                    ptr.sz  = sz;
                    ScnArraySorted_addPtr(opq->ctx, &opq->ptrs, &ptr, STScnMemBlockPtr);
                    //remove/modify gap
                    if(g->sz == sz){
                        //remove gap
                        ScnArraySorted_removeItemAtIndex(&opq->gaps, (ScnSI32)(g - gStart));
                    } else {
                        //edit gap
                        g->iStart   += sz;
                        g->sz       -= sz;
                    }
                    SCN_ASSERT(opq->state.szAvail >= sz)
                    opq->state.szAvail -= sz;
                    r.idx   = (ScnUI32)((ScnBYTE*)ptr.ptr - opq->chunk.ptr);
                    r.ptr   = ptr.ptr;
                    break;
                }
                //next
                g++;
            }
            //keep track of last failed 'ScnMemBlock_malloc'
            if(r.ptr == NULL && opq->state.szSmallestMallocFailed > sz){
                opq->state.szSmallestMallocFailed = sz;
            }
#           ifdef SCN_ASSERTS_ACTIVATED
            //TMP
            ScnMemBlock_validateIndexLockepOpq_(opq);
#           endif
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
    
}

ScnBOOL ScnMemBlock_mfree(ScnMemBlockRef ref, const STScnAbsPtr ptr){
    ScnBOOL r = ScnFALSE;
    STScnMemBlockOpq* opq = (STScnMemBlockOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        //search ptr
        STScnMemBlockPtr srchPtr = STScnMemBlockPtr_Zero;
        srchPtr.ptr = (void*)ptr.ptr;
        const ScnSI32 iFnd = ScnArraySorted_indexOf(&opq->ptrs, &srchPtr);
        if(iFnd < 0){
            //not found
            SCN_ASSERT(ScnFALSE);
        } else {
            STScnMemBlockPtr* fndPtr = &opq->ptrs.arr[iFnd];
            SCN_ASSERT(fndPtr->ptr == ptr.ptr)
            //find gap
            {
                ScnUI32 newGapSz = fndPtr->sz;
                STScnMemBlockGap* gStart = opq->gaps.arr;
                STScnMemBlockGap srchGap = STScnMemBlockGap_Zero;
                srchGap.iStart = (ScnUI32)((ScnBYTE*)ptr.ptr - opq->chunk.ptr);
                const ScnSI32 iNxtGap = ScnArraySorted_indexForNew(&opq->gaps, &srchGap);
                if(iNxtGap < opq->gaps.use && ((ScnBYTE*)ptr.ptr + fndPtr->sz) == (opq->chunk.ptr + gStart[iNxtGap].iStart)){
                    //merge new gap with next gap
                    gStart[iNxtGap].iStart  -= fndPtr->sz;
                    gStart[iNxtGap].sz      += fndPtr->sz;
                    newGapSz                = gStart[iNxtGap].sz;
                } else if(iNxtGap > 0 && (opq->chunk.ptr + gStart[iNxtGap - 1].iStart + gStart[iNxtGap  - 1].sz) == (ScnBYTE*)ptr.ptr){
                    //merge new gap with prev gap
                    gStart[iNxtGap - 1].sz  += fndPtr->sz;
                    newGapSz                = gStart[iNxtGap - 1].sz;
                } else {
                    //create new gap
                    STScnMemBlockGap gap = STScnMemBlockGap_Zero;
                    gap.iStart  = (ScnUI32)((ScnBYTE*)ptr.ptr - opq->chunk.ptr);
                    gap.sz      = fndPtr->sz;
                    ScnArraySorted_addPtr(opq->ctx, &opq->gaps, &gap, STScnMemBlockGap);
                }
                SCN_ASSERT(opq->state.szAvail + fndPtr->sz <= (opq->chunk.rngAfterEnd - opq->chunk.rngStart));
                opq->state.szAvail += fndPtr->sz;
                //
                if(opq->state.szSmallestMallocFailed <= newGapSz){
                    opq->state.szSmallestMallocFailed = 0;
                }
            }
            //remove ptr
            ScnArraySorted_removeItemAtIndex(&opq->ptrs, iFnd);
#           ifdef SCN_ASSERTS_ACTIVATED
            //TMP
            //ScnMemBlock_validateIndexLockepOpq_(opq);
#           endif
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnUI32 ScnMemBlock_mAvailSz(ScnMemBlockRef ref){
    STScnMemBlockOpq* opq = (STScnMemBlockOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->state.szAvail;
}

void ScnMemBlock_prepareForNewMallocsActions(ScnMemBlockRef ref, const ScnUI32 ammActions){   //increases the index sz
    STScnMemBlockOpq* opq = (STScnMemBlockOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        ScnArraySorted_prepareForGrowth(opq->ctx, &opq->ptrs, ammActions, STScnMemBlockPtr);
    }
    ScnMutex_unlock(opq->mutex);
}

void ScnMemBlock_clear(ScnMemBlockRef ref){ //clears the index, all pointers are invalid after this call
    STScnMemBlockOpq* opq = (STScnMemBlockOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        //set initial state
        ScnArraySorted_empty(&opq->ptrs);
        ScnArraySorted_empty(&opq->gaps);
        //reset state
        ScnMemory_setZeroSt(opq->state, STScnMemBlockState);
        //register the whole range as the initial gap
        if(opq->chunk.rngStart < opq->chunk.rngAfterEnd){
            STScnMemBlockGap gap = STScnMemBlockGap_Zero;
            gap.iStart  = opq->chunk.rngStart;
            gap.sz      = opq->chunk.rngAfterEnd - opq->chunk.rngStart;
            ScnArraySorted_addPtr(opq->ctx, &opq->gaps, &gap, STScnMemBlockGap);
            opq->state.szAvail = gap.sz;
        }
    }
    ScnMutex_unlock(opq->mutex);
}

//dgb

ScnBOOL ScnMemBlock_validateIndexLockepOpq_(STScnMemBlockOpq* opq){
    ScnBOOL r = ScnFALSE;
    ScnUI32 gapsTotalSz = 0, gapLargestSz = 0, ptrsTotalSz = 0;
    //
    ScnBYTE* ptr = opq->chunk.ptr + opq->chunk.rngStart;
    const ScnBYTE* ptrAfterEnd = opq->chunk.ptr + opq->chunk.rngAfterEnd;
    //
    STScnMemBlockGap* gStart = opq->gaps.arr;
    STScnMemBlockGap* g = gStart;
    const STScnMemBlockGap* gAfterEnd = g + opq->gaps.use;
    //
    STScnMemBlockPtr* pStart = opq->ptrs.arr;
    STScnMemBlockPtr* p = pStart;
    const STScnMemBlockPtr* pAfterEnd = p + opq->ptrs.use;
    //walk the pointer ahead
    while(g < gAfterEnd || p < pAfterEnd || ptr < ptrAfterEnd){
        if(g < gAfterEnd && (opq->chunk.ptr + g->iStart) == ptr){
            //a gap
            if(gapLargestSz < g->sz) gapLargestSz = g->sz;
            gapsTotalSz += g->sz;
            ptr += g->sz;
            g++;
        } else if(p < pAfterEnd && p->ptr == ptr){
            //an allocated pointer
            ptrsTotalSz += p->sz;
            ptr += p->sz;
            p++;
        } else {
#           ifdef SCN_ASSERTS_ACTIVATED
            const ScnSI64 pos = (ScnSI64)(ptr - opq->chunk.ptr);
            const ScnSI64 deltaGap = (g < gAfterEnd ? (ScnSI64)(opq->chunk.ptr + g->iStart) - (ScnSI64)ptr : -1);
            const ScnSI64 deltaPtr = (p < pAfterEnd ? (ScnSI64)p->ptr - (ScnSI64)ptr : -1);
            SCN_PRINTF_ERROR("ScnMemBlock_validateIndexLockepOpq_ failes: pos %lld/%lld (%lld to next gap, %lld to next used-ptr).\n", pos, (ScnSI64)opq->chunk.rngAfterEnd, deltaGap, deltaPtr);
#           endif
            SCN_ASSERT(ScnFALSE); //indexes are not valid
            break;
        }
    }
    SCN_ASSERT(gapsTotalSz + ptrsTotalSz == (opq->chunk.rngAfterEnd - opq->chunk.rngStart))
    SCN_ASSERT(g == gAfterEnd && p == pAfterEnd && ptr == ptrAfterEnd)
    SCN_ASSERT(gapsTotalSz == opq->state.szAvail && (opq->state.szSmallestMallocFailed == 0 || gapLargestSz < opq->state.szSmallestMallocFailed))
    if(gapsTotalSz + ptrsTotalSz == (opq->chunk.rngAfterEnd - opq->chunk.rngStart) && g == gAfterEnd && p == pAfterEnd && ptr == ptrAfterEnd && gapsTotalSz == opq->state.szAvail && (opq->state.szSmallestMallocFailed == 0 || gapLargestSz < opq->state.szSmallestMallocFailed)){
        r = ScnTRUE;
    }
    return r;
}
    
ScnBOOL ScnMemBlock_validateIndex(ScnMemBlockRef ref){
    ScnBOOL r = ScnFALSE;
    STScnMemBlockOpq* opq = (STScnMemBlockOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        r = ScnMemBlock_validateIndexLockepOpq_(opq);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}
