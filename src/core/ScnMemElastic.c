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

typedef struct STScnMemElasticState_ {
    ScnUI32        idxsTotalSz;       //iOffset for next block
} STScnMemElasticState;

// STScnMemElasticBlock

#define STScnMemElasticBlock_Zero  { 0, 0, 0, ScnObjRef_Zero }

typedef struct STScnMemElasticBlock_ {
    ScnUI32         iOffset;    //idx-at-block + iOffset = abstract-idx-at-blocks
    ScnUI32         idxsSz;     //ammount of addreses from idx-0
    ScnUI32         szSmallestMallocFailed; //for 'ScnMemBlock_malloc' quick-ignores
    ScnMemBlockRef  block;      //memory data
} STScnMemElasticBlock;

//STScnMemElasticOpq

typedef struct STScnMemElasticOpq_ {
    ScnContextRef     ctx;
    ScnMutexRef       mutex;
    STScnMemElasticCfg   cfg;
    STScnMemElasticState state;
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
    ScnContext_releaseAndNullify(&opq->ctx);
}

//

ScnBOOL ScnMemElastic_prepare(ScnMemElasticRef ref, const STScnMemElasticCfg* cfg, ScnUI32* optDstBlocksTotalSz){
    ScnBOOL r = ScnFALSE;
    STScnMemElasticOpq* opq = (STScnMemElasticOpq*)ScnSharedPtr_getOpq(ref.ptr);
    //
    ScnMutex_lock(opq->mutex);
    if(cfg != NULL && opq->blocks.use == 0){
        STScnMemBlockCfg bCfg = STScnMemBlockCfg_Zero;
        bCfg.sizeAlign   = (cfg->sizeAlign > 0 ? cfg->sizeAlign : 4);   //whole memory block size alignment
        bCfg.idxsAlign   = (cfg->idxsAlign > 0 ? cfg->idxsAlign : 4);   //individual pointers alignment
        bCfg.size = (cfg->sizePerBlock + bCfg.idxsAlign - 1) / bCfg.idxsAlign * bCfg.idxsAlign;
        bCfg.idxZeroIsValid = (opq->blocks.use == 0 ? cfg->idxZeroIsValid : ScnTRUE);
        //copy cfg
        {
            opq->cfg = *cfg;
            //ScnStruct_stRelease(ScnMemElasticCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
            //ScnStruct_stClone(ScnMemElasticCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
            opq->cfg.sizeAlign = bCfg.sizeAlign;
            opq->cfg.idxsAlign = bCfg.idxsAlign;
            opq->cfg.sizePerBlock = bCfg.size;
        }
        //intial state
        {
            r = ScnTRUE;
            opq->state.idxsTotalSz = 0;
            //allocate initial blocks
            if(bCfg.size > 0){
                ScnUI32 usableSzInit = (((cfg->sizeInitial + bCfg.idxsAlign - 1) / bCfg.idxsAlign * bCfg.idxsAlign) + bCfg.size - 1) / bCfg.size * bCfg.size;
                while(usableSzInit > 0){
                    STScnAbsPtr ptrAfterEnd = STScnAbsPtr_Zero;
                    STScnMemElasticBlock b = STScnMemElasticBlock_Zero;
                    b.iOffset   = opq->state.idxsTotalSz;
                    b.idxsSz    = bCfg.size;
                    b.block     = ScnMemBlock_alloc(opq->ctx);
                    if(!ScnMemBlock_prepare(b.block, &bCfg, &ptrAfterEnd)){
                        ScnMemBlock_release(&b.block);
                        r = ScnFALSE;
                        break;
                    } else {
                        b.idxsSz = ptrAfterEnd.idx;
                        opq->state.idxsTotalSz += ptrAfterEnd.idx;
                        ScnArray_addValue(opq->ctx, &opq->blocks, b, STScnMemElasticBlock);
                    }
                    usableSzInit -= bCfg.size;
                }
                opq->cfg.sizeInitial = usableSzInit;
            }
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
            ScnUI32 idxSz = ((usableSz > opq->cfg.sizePerBlock ? usableSz : opq->cfg.sizePerBlock) + opq->cfg.idxsAlign - 1) / opq->cfg.idxsAlign * opq->cfg.idxsAlign;
            //apply size limitation
            if(opq->cfg.sizeMax > 0 && opq->state.idxsTotalSz >= opq->cfg.sizeMax){
                //already reached limit
            } else {
                //allocate new block
                STScnMemElasticBlock b = STScnMemElasticBlock_Zero;
                b.idxsSz    = (opq->cfg.sizeMax == 0 || (opq->state.idxsTotalSz + idxSz) <= opq->cfg.sizeMax ? idxSz : opq->cfg.sizeMax - opq->state.idxsTotalSz );
                b.iOffset   = opq->state.idxsTotalSz;
                b.block     = ScnMemBlock_alloc(opq->ctx);
                {
                    STScnMemBlockCfg bCfg = STScnMemBlockCfg_Zero;
                    bCfg.sizeAlign   = opq->cfg.sizeAlign;   //whole memory block size alignment
                    bCfg.idxsAlign   = opq->cfg.idxsAlign;   //individual pointers alignment
                    bCfg.size = b.idxsSz;
                    bCfg.idxZeroIsValid = (opq->blocks.use == 0 ? opq->cfg.idxZeroIsValid : ScnTRUE);
                    STScnAbsPtr ptrAfterEnd = STScnAbsPtr_Zero;
                    if(!ScnMemBlock_prepare(b.block, &bCfg, &ptrAfterEnd)){
                        ScnMemBlock_release(&b.block);
                    } else {
                        r = ScnMemBlock_malloc(b.block, usableSz);
                        if(r.ptr == NULL){
                            SCN_ASSERT(ScnFALSE) //program-logic error
                            ScnMemBlock_release(&b.block);
                        } else {
                            //convert block-index to blocks-index
                            r.idx += b.iOffset;
                            b.idxsSz = ptrAfterEnd.idx;
                            opq->state.idxsTotalSz += ptrAfterEnd.idx;
                            ScnArray_addValue(opq->ctx, &opq->blocks, b, STScnMemElasticBlock);
                        }
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

ScnBOOL ScnMemElastic_validateIndexLockedOpq_(STScnMemElasticOpq* opq){
    ScnBOOL r = ScnTRUE;
    {
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
