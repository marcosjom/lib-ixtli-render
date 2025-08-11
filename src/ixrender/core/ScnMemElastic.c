//
//  ScnMemElastic.c
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/core/ScnMemElastic.h"
#include "ixrender/core/ScnArray.h"

//STScnMemElasticState

#define STScnMemElasticState_Zero   { 0 }

typedef struct STScnMemElasticState {
    ScnUI32        idxsTotalSz;       //iOffset for next block
} STScnMemElasticState;

// STScnMemElasticBlock

#define STScnMemElasticBlock_Zero  { 0, 0, 0, ScnObjRef_Zero }

typedef struct STScnMemElasticBlock {
    ScnUI32         iOffset;    //idx-at-block + iOffset = abstract-idx-at-blocks
    ScnUI32         idxsSz;     //ammount of addreses from idx-0
    ScnUI32         szSmallestMallocFailed; //for 'ScnMemBlock_malloc' quick-ignores
    ScnMemBlockRef  block;      //memory data
} STScnMemElasticBlock;

//STScnMemElasticOpq

typedef struct STScnMemElasticOpq {
    ScnContextRef           ctx;
    ScnMutexRef             mutex;
    STScnMemElasticCfg      cfg;
    STScnMemElasticState    state;
    ScnArrayStruct(blocks, STScnMemElasticBlock);
} STScnMemElasticOpq;

//

ScnSI32 ScnMemElastic_getOpqSz(void){
    return (ScnSI32)sizeof(STScnMemElasticOpq);
}

void ScnMemElastic_initZeroedOpq(ScnContextRef ctx, void* obj) {
    STScnMemElasticOpq* opq = (STScnMemElasticOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex = ScnContext_allocMutex(opq->ctx);
    //
    ScnArray_init(opq->ctx, &opq->blocks, 0, 4, STScnMemElasticBlock);
}

void ScnMemElastic_destroyOpq(void* obj){
    STScnMemElasticOpq* opq = (STScnMemElasticOpq*)obj;
    //blocks
    {
        STScnMemElasticBlock* b = opq->blocks.arr;
        const STScnMemElasticBlock* bAfterEnd = b + opq->blocks.use;
        while(b < bAfterEnd){
            ScnMemBlock_release(&b->block);
            ++b;
        }
        ScnArray_empty(&opq->blocks);
        ScnArray_destroy(opq->ctx, &opq->blocks);
    }
    //state
    {
        //nothing
    }
    //config
    {
        opq->cfg = (STScnMemElasticCfg)STScnMemElasticCfg_Zero;
        //ScnStruct_stRelease(ScnMemElasticCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
    }
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNull(&opq->ctx);
}

//

ScnUI64 ScnMemElastic_gcd_(const ScnUI64 a, const ScnUI64 b) {
    if (b == 0) return a;
    return ScnMemElastic_gcd_(b, a % b);
}

ScnUI32 ScnMemElastic_lcm_(const ScnUI32 a, const ScnUI32 b) {
    return (ScnUI32)((ScnUI64)a / ScnMemElastic_gcd_(a, b)) * b;
}

ScnBOOL ScnMemElastic_prepare(ScnMemElasticRef ref, const STScnMemElasticCfg* cfg, ScnUI32* optDstBlocksTotalSz){
    ScnBOOL r = ScnFALSE;
    STScnMemElasticOpq* opq = (STScnMemElasticOpq*)ScnSharedPtr_getOpq(ref.ptr);
    //
    ScnMutex_lock(opq->mutex);
    if(cfg != NULL && opq->blocks.use == 0){
        const ScnUI32 sizeAlign = (cfg->sizeAlign > 0 ? cfg->sizeAlign : 4);   //whole memory block size alignment
        const ScnUI32 idxsAlign = (cfg->idxsAlign > 0 ? cfg->idxsAlign : 4);   //individual pointers alignment
        const ScnUI32 szPerBlck = ScnMemElastic_lcm_(((cfg->sizePerBlock + idxsAlign - 1) / idxsAlign * idxsAlign), sizeAlign);
        ScnUI32 curUsableSz = 0;
        r = ScnTRUE;
        //add blocks until usableSz is populated
        while(curUsableSz < cfg->sizeInitial){
            ScnMemBlockRef block = ScnMemBlock_alloc(opq->ctx);
            STScnAbsPtr ptrAfterEnd = STScnAbsPtr_Zero;
            STScnMemBlockCfg bCfg = STScnMemBlockCfg_Zero;
            bCfg.size       = szPerBlck;
            bCfg.sizeAlign  = sizeAlign;
            bCfg.idxsAlign  = idxsAlign;
            bCfg.idxZeroIsValid = (opq->blocks.use == 0 ? cfg->idxZeroIsValid : ScnTRUE);
            if(ScnMemBlock_isNull(block) || !ScnMemBlock_prepare(block, &bCfg, &ptrAfterEnd)){
                ScnMemBlock_releaseAndNull(&block);
                r = ScnFALSE;
                break;
            } else {
                STScnMemElasticBlock b = STScnMemElasticBlock_Zero;
                b.iOffset   = opq->state.idxsTotalSz;
                b.idxsSz    = ptrAfterEnd.idx;
                b.block     = block;
                if(NULL == ScnArray_addPtr(opq->ctx, &opq->blocks, &b, STScnMemElasticBlock)){
                    ScnMemBlock_releaseAndNull(&block);
                    r = ScnFALSE;
                    break;
                } else {
                    SCN_ASSERT(ptrAfterEnd.idx >= bCfg.idxsAlign) //at least one record
                    SCN_ASSERT((ptrAfterEnd.idx % bCfg.sizeAlign) == 0) //size aligned
                    curUsableSz += ptrAfterEnd.idx - (bCfg.idxZeroIsValid ? 0 : bCfg.idxsAlign);
                    opq->state.idxsTotalSz += ptrAfterEnd.idx;
                    SCN_PRINTF_INFO("ScnMemElastic_prepare, added STScnMemElasticBlock(+%u bytes, %u total).\n", ptrAfterEnd.idx, opq->state.idxsTotalSz);
                }
            }
        }
        //set
        if(r){
            opq->cfg                = *cfg;
            opq->cfg.sizeAlign      = sizeAlign;
            opq->cfg.idxsAlign      = idxsAlign;
            opq->cfg.sizePerBlock   = szPerBlck;
            opq->cfg.sizeInitial    = curUsableSz;
        }
        if(optDstBlocksTotalSz != NULL){
            *optDstBlocksTotalSz = opq->state.idxsTotalSz;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnMemElastic_hasPtrs(ScnMemElasticRef ref){ //allocations made?
    ScnBOOL r = ScnFALSE;
    STScnMemElasticOpq* opq = (STScnMemElasticOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnMemElasticBlock* b = opq->blocks.arr;
        const STScnMemElasticBlock* bAfterEnd = b + opq->blocks.use;
        while(b < bAfterEnd){
            if(ScnMemBlock_hasPtrs(b->block)){
                r = ScnTRUE;
                break;
            }
            ++b;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnUI32 ScnMemElastic_getAddressableSize(ScnMemElasticRef ref){ //includes the address zero
    ScnUI32 r = 0;
    STScnMemElasticOpq* opq = (STScnMemElasticOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnMemElasticBlock* b = opq->blocks.arr;
        const STScnMemElasticBlock* bAfterEnd = b + opq->blocks.use;
        while(b < bAfterEnd){
            SCN_ASSERT(b->iOffset == r)
            r += b->idxsSz;
            ++b;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

STScnRangeU ScnMemElastic_getUsedAddressesRngLockedOpq_(STScnMemElasticOpq* opq){
    STScnRangeU r = STScnRangeU_Zero;
    if(opq->blocks.use > 0){
        STScnRangeU rngLft          = STScnRangeU_Zero;
        STScnRangeU rngRght         = STScnRangeU_Zero;
        STScnMemElasticBlock* bLft  = opq->blocks.arr;
        STScnMemElasticBlock* bAfterEnd = opq->blocks.arr + opq->blocks.use;
        //find first used ptr
        while(bLft < bAfterEnd && rngLft.size <= 0){
            rngLft          = ScnMemBlock_getUsedAddressesRng(bLft->block);
            rngLft.start    += bLft->iOffset;
            ++bLft;
        }
        //find last used ptr
        while(bLft < bAfterEnd && rngRght.size <= 0){
            --bAfterEnd;
            rngRght         = ScnMemBlock_getUsedAddressesRng(bAfterEnd->block);
            rngRght.start   += bAfterEnd->iOffset;
        }
        if(rngRght.size == 0){
            rngRght = rngLft;
        }
        //elastic used rng
        if(rngLft.size > 0 && rngRght.size > 0){
            r.start = rngLft.start;
            r.size  = rngRght.start + rngRght.size - r.start;
        }
    }
    return r;
}
                                              

STScnRangeU ScnMemElastic_getUsedAddressesRng(ScnMemElasticRef ref){ //highest allocated address index
    STScnRangeU r = STScnRangeU_Zero;
    STScnMemElasticOpq* opq = (STScnMemElasticOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        r = ScnMemElastic_getUsedAddressesRngLockedOpq_(opq);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

STScnRangeU ScnMemElastic_getUsedAddressesRngAligned(ScnMemElasticRef ref){ //range that covers all allocated addresses index
    STScnRangeU r = STScnRangeU_Zero;
    STScnMemElasticOpq* opq = (STScnMemElasticOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnRangeU rr = ScnMemElastic_getUsedAddressesRngLockedOpq_(opq);
        if(rr.size > 0){
            r.start   = rr.start / opq->cfg.sizeAlign * opq->cfg.sizeAlign;
            r.size    = ((rr.start + rr.size + opq->cfg.sizeAlign - 1) / opq->cfg.sizeAlign * opq->cfg.sizeAlign) - r.start;
            SCN_ASSERT(r.start <= rr.start && (rr.start + rr.size) <= (r.start + r.size))
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

STScnAbsPtr ScnMemElastic_getNextContinuousAddress(ScnMemElasticRef ref, const ScnUI32 iAddress, ScnUI32* dstContinuousSz){
    STScnAbsPtr r = STScnAbsPtr_Zero; ScnUI32 continuousSz = 0;
    STScnMemElasticOpq* opq = (STScnMemElasticOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(iAddress == 0){
        //frist block
        STScnMemElasticBlock* b = opq->blocks.arr;
        const STScnMemElasticBlock* bAfterEnd = b + opq->blocks.use;
        if(b < bAfterEnd){
            r = ScnMemBlock_getStarAddress(b->block);
            continuousSz = b->idxsSz;
        }
    } else {
        //next block
        ScnUI32 iPos = 0;
        STScnMemElasticBlock* b = opq->blocks.arr;
        const STScnMemElasticBlock* bAfterEnd = b + opq->blocks.use;
        while(b < bAfterEnd){
            SCN_ASSERT(b->iOffset == iPos)
            iPos += b->idxsSz;
            if(iAddress < iPos){
                const STScnAbsPtr ba = ScnMemBlock_getStarAddress(b->block);
                continuousSz = iPos - iAddress;
                r.idx = iAddress;
                r.ptr = &((ScnBYTE*)ba.ptr)[b->idxsSz - continuousSz];
                break;
            }
            ++b;
        }
    }
    ScnMutex_unlock(opq->mutex);
    if(dstContinuousSz != NULL){
        *dstContinuousSz = continuousSz;
    }
    return r;
}

//allocations

STScnAbsPtr ScnMemElastic_malloc(ScnMemElasticRef ref, const ScnUI32 usableSz, ScnUI32* optDstBlocksTotalSz){
    STScnAbsPtr r = STScnAbsPtr_Zero;
    STScnMemElasticOpq* opq = (STScnMemElasticOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->cfg.idxsAlign > 0){
        //try on current blocks
        STScnMemElasticBlock* b = opq->blocks.arr;
        const STScnMemElasticBlock* bAfterEnd = b + opq->blocks.use;
        while(b < bAfterEnd){
            if(b->szSmallestMallocFailed == 0 || usableSz < b->szSmallestMallocFailed){ //skip, already failed to allocate
                //try to allocate
                r = ScnMemBlock_malloc(b->block, usableSz);
                if(r.ptr == NULL){
                    b->szSmallestMallocFailed = usableSz;
                } else {
                    //convert block-index to blocks-index
                    r.idx += b->iOffset;
                    break;
                }
            }
            ++b;
        }
        //create new block
        if(b >= bAfterEnd){
            ScnMemBlockRef block = ScnMemBlock_alloc(opq->ctx);
            STScnAbsPtr ptrAfterEnd = STScnAbsPtr_Zero;
            STScnMemBlockCfg bCfg = STScnMemBlockCfg_Zero;
            bCfg.idxZeroIsValid = (opq->blocks.use == 0 ? opq->cfg.idxZeroIsValid : ScnTRUE);
            bCfg.sizeAlign  = opq->cfg.sizeAlign;
            bCfg.idxsAlign  = opq->cfg.idxsAlign;
            bCfg.size       = ScnMemElastic_lcm_(((usableSz + (bCfg.idxZeroIsValid ? 0 : bCfg.idxsAlign) + bCfg.idxsAlign - 1) / bCfg.idxsAlign * bCfg.idxsAlign), bCfg.sizeAlign);
            if(ScnMemBlock_isNull(block) || !ScnMemBlock_prepare(block, &bCfg, &ptrAfterEnd)){
                ScnMemBlock_releaseAndNull(&block);
            } else {
                STScnMemElasticBlock b = STScnMemElasticBlock_Zero;
                b.iOffset   = opq->state.idxsTotalSz;
                b.idxsSz    = ptrAfterEnd.idx;
                b.block     = block;
                if(NULL == ScnArray_addPtr(opq->ctx, &opq->blocks, &b, STScnMemElasticBlock)){
                    ScnMemBlock_releaseAndNull(&block);
                } else {
                    SCN_ASSERT(ptrAfterEnd.idx >= bCfg.idxsAlign) //at least one record
                    SCN_ASSERT((ptrAfterEnd.idx % bCfg.sizeAlign) == 0) //size aligned
                    opq->state.idxsTotalSz += ptrAfterEnd.idx;
                    SCN_PRINTF_INFO("ScnMemElastic_malloc, added STScnMemElasticBlock(+%u bytes, %u total).\n", ptrAfterEnd.idx, opq->state.idxsTotalSz);
                    //
                    r = ScnMemBlock_malloc(b.block, usableSz);
                    if(r.ptr == NULL){
                        SCN_ASSERT(ScnFALSE) //program-logic error
                    } else {
                        SCN_ASSERT((ptrAfterEnd.idx % bCfg.sizeAlign) == 0)
                        //convert block-index to blocks-index
                        r.idx += b.iOffset;
                    }
                }
            }
        }
        //
        if(optDstBlocksTotalSz != NULL){
            *optDstBlocksTotalSz = opq->state.idxsTotalSz;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnMemElastic_mfree(ScnMemElasticRef ref, const STScnAbsPtr ptr){
    ScnBOOL r = ScnFALSE;
    STScnMemElasticOpq* opq = (STScnMemElasticOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->cfg.idxsAlign > 0){
        //find the idx-at-block of this abstract-idx-at-blocks
        STScnAbsPtr ptr2 = ptr;
        STScnMemElasticBlock* b = opq->blocks.arr;
        const STScnMemElasticBlock* bAfterEnd = b + opq->blocks.use;
        while(b < bAfterEnd){
            if(ptr2.idx < b->idxsSz){
                r = ScnMemBlock_mfree(b->block, ptr2);
                if(r){
                    b->szSmallestMallocFailed = 0;
                }
                break;
            }
            SCN_ASSERT(ptr2.idx >= b->idxsSz)
            ptr2.idx -= b->idxsSz;
            ++b;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//

void ScnMemElastic_clear(ScnMemElasticRef ref){ //clears the index, all pointers are invalid after this call
    STScnMemElasticOpq* opq = (STScnMemElasticOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnMemElasticBlock* b = opq->blocks.arr;
        const STScnMemElasticBlock* bAfterEnd = b + opq->blocks.use;
        while(b < bAfterEnd){
            ScnMemBlock_clear(b->block);
            b->szSmallestMallocFailed = 0;
            ++b;
        }
    }
    ScnMutex_unlock(opq->mutex);
}

//dbg

ScnBOOL ScnMemElastic_pushPtrs(ScnMemElasticRef ref, STScnMemPushPtrsItf* itf, void* itfParam){
    ScnBOOL r = ScnFALSE;
    STScnMemElasticOpq* opq = (STScnMemElasticOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnMemElasticBlock* b = opq->blocks.arr;
        const STScnMemElasticBlock* bAfterEnd = b + opq->blocks.use;
        r = ScnTRUE;
        while(b < bAfterEnd){
            if(!ScnMemBlock_pushPtrs(b->block, b->iOffset, itf, itfParam)){
                r = ScnFALSE;
                break;
            }
            ++b;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnMemElastic_validateIndexLockedOpq_(STScnMemElasticOpq* opq){
    ScnBOOL r = ScnTRUE;
    STScnMemElasticBlock* b = opq->blocks.arr;
    const STScnMemElasticBlock* bAfterEnd = b + opq->blocks.use;
    ScnUI32 idxsTotalSz = 0;
    while(b < bAfterEnd){
        if(b->iOffset != idxsTotalSz){
            r = ScnFALSE;
        } else if(!ScnMemBlock_validateIndex(b->block)){
            r = ScnFALSE;
        }
        idxsTotalSz += b->idxsSz;
        ++b;
    }
    //
    if(idxsTotalSz != opq->state.idxsTotalSz){
        r = ScnFALSE;
    }
    return r;
}
    
ScnBOOL ScnMemElastic_validateIndex(ScnMemElasticRef ref){
    ScnBOOL r = ScnTRUE;
    STScnMemElasticOpq* opq = (STScnMemElasticOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        r = ScnMemElastic_validateIndexLockedOpq_(opq);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}
