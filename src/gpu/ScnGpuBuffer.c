//
//  ScnGpuBuffer.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/gpu/ScnGpuBuffer.h"
#include "ixrender/core/ScnArraySorted.h"

//STScnGpuBufferOpq

typedef struct STScnGpuBufferOpq_ {
    STScnContextRef     ctx;
    STScnMutexRef       mutex;
    //
    STScnGpuBufferCfg   cfg;    //config
    STScnMemElasticRef  mem;    //memory
    //state
    struct {
        ScnUI32         totalSzLast;
    } state;
    //changes
    struct {
        ScnBOOL         size;   //buffer size changed
        ScnArraySortedStruct(rngs, STScnRangeU);
    } changes;
    //api
    struct {
        STScnGpuBufferApiItf itf;
        void*           itfParam;
        void*           data;
    } api;
} STScnGpuBufferOpq;

ScnSI32 ScnGpuBuffer_getOpqSz(void){
    return (ScnSI32)sizeof(STScnGpuBufferOpq);
}

void ScnGpuBuffer_initZeroedOpq(STScnContextRef ctx, void* obj) {
    STScnGpuBufferOpq* opq = (STScnGpuBufferOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex = ScnContext_mutex_alloc(opq->ctx);
    //changes
    {
        ScnArraySorted_init(opq->ctx, &opq->changes.rngs, 0, 128, STScnRangeU, ScnCompare_STScnRangeU);
    }
}

void ScnGpuBuffer_destroyOpq(void* obj){
    STScnGpuBufferOpq* opq = (STScnGpuBufferOpq*)obj;
    //api
    {
        if(opq->api.data != NULL){
            if(opq->api.itf.destroy != NULL){
                (*opq->api.itf.destroy)(opq->api.data, opq->api.itfParam);
            }
            opq->api.data = NULL;
        }
        ScnMemory_setZeroSt(opq->api.itf, STScnGpuBufferApiItf);
        opq->api.itfParam = NULL;
    }
    //
    if(!ScnMemElastic_isNull(opq->mem)){
        ScnMemElastic_release(&opq->mem);
        ScnMemElastic_null(&opq->mem);
    }
    //ScnStruct_stRelease(ScnGpuBufferCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
    //changes
    {
        ScnArraySorted_destroy(opq->ctx, &opq->changes.rngs);
    }
    //
    if(!ScnMutex_isNull(opq->mutex)){
        ScnMutex_free(&opq->mutex);
        ScnMutex_null(&opq->mutex);
    }
    //
    if(!ScnContext_isNull(opq->ctx)){
        ScnContext_release(&opq->ctx);
        ScnContext_null(&opq->ctx);
    }
}

//

ScnBOOL ScnGpuBuffer_prepare(STScnGpuBufferRef ref, const STScnGpuBufferCfg* cfg, const STScnGpuBufferApiItf* itf, void* itfParam) {
    ScnBOOL r = ScnFALSE;
    STScnGpuBufferOpq* opq = (STScnGpuBufferOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(cfg != NULL && ScnMemElastic_isNull(opq->mem) && itf != NULL && itf->create != NULL && itf->destroy != NULL){
        void* data = (*itf->create)(cfg, itfParam);
        if(data != NULL){
            ScnUI32 totalSz = 0;
            STScnMemElasticRef mem = ScnMemElastic_alloc(opq->ctx);
            if(ScnMemElastic_prepare(mem, &cfg->mem, &totalSz)){
                //set
                ScnMemElastic_set(&opq->mem, mem);
                ScnMemElastic_null(&mem); //consume
                //
                opq->cfg = *cfg;
                //ScnStruct_stRelease(ScnGpuBufferCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
                //ScnStruct_stClone(ScnGpuBufferCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
                //state
                {
                    opq->state.totalSzLast = totalSz;
                }
                //changes
                {
                    opq->changes.size = ScnTRUE;
                    ScnArraySorted_empty(&opq->changes.rngs);
                }
                //api
                {
                    if(opq->api.data != NULL){
                        if(opq->api.itf.destroy != NULL){
                            (*opq->api.itf.destroy)(opq->api.data, opq->api.itfParam);
                        }
                        opq->api.data = NULL;
                    }
                    ScnMemory_setZeroSt(opq->api.itf, STScnGpuBufferApiItf);
                    opq->api.itfParam = NULL;
                    //
                    if(itf != NULL){
                        opq->api.itf = *itf;
                        opq->api.itfParam = itfParam;
                    }
                    //data
                    opq->api.data = data; data = NULL; //consume
                }
                r = ScnTRUE;
            }
            //relese (if not consumed)
            if(!ScnMemElastic_isNull(mem)){
                ScnMemElastic_release(&mem);
                ScnMemElastic_null(&mem);
            }
        }
        //destroy (if not consumed)
        if(data != NULL && itf->destroy != NULL){
            (*itf->destroy)(data, itfParam);
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnGpuBuffer_clear(STScnGpuBufferRef ref){
    ScnBOOL r = ScnFALSE;
    STScnGpuBufferOpq* opq = (STScnGpuBufferOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnMemElastic_isNull(opq->mem)){
        ScnMemElastic_clear(opq->mem);
        r = ScnTRUE;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

STScnAbsPtr ScnGpuBuffer_malloc(STScnGpuBufferRef ref, const ScnUI32 usableSz){
    STScnAbsPtr r = STScnAbsPtr_Zero;
    STScnGpuBufferOpq* opq = (STScnGpuBufferOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnMemElastic_isNull(opq->mem)){
        ScnUI32 totalSz = 0;
        r = ScnMemElastic_malloc(opq->mem, usableSz, &totalSz);
        if(opq->state.totalSzLast != totalSz){
            opq->state.totalSzLast = totalSz;
            //changes
            opq->changes.size = ScnTRUE;
            ScnArraySorted_empty(&opq->changes.rngs);
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnGpuBuffer_mfree(STScnGpuBufferRef ref, const STScnAbsPtr ptr){
    ScnBOOL r = ScnFALSE;
    STScnGpuBufferOpq* opq = (STScnGpuBufferOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnMemElastic_isNull(opq->mem)){
        r = ScnMemElastic_mfree(opq->mem, ptr);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnGpuBuffer_mInvalidate(STScnGpuBufferRef ref, const STScnAbsPtr ptr, const ScnUI32 sz){
    ScnBOOL r = ScnFALSE;
    STScnGpuBufferOpq* opq = (STScnGpuBufferOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnMemElastic_isNull(opq->mem) && opq->cfg.mem.sizeAlign > 0){
        if(sz > 0 && !opq->changes.size){ //no need to evaluate rngs if the whole buffer requires an update
            STScnRangeU rng = STScnRangeU_Zero;
            rng.start  = ptr.idx / opq->cfg.mem.sizeAlign * opq->cfg.mem.sizeAlign;
            rng.size      = ((ptr.idx + sz + opq->cfg.mem.sizeAlign - 1) / opq->cfg.mem.sizeAlign * opq->cfg.mem.sizeAlign) - rng.start;
            SCN_ASSERT(rng.start <= ptr.idx && (ptr.idx + sz) <= (rng.start + rng.size))
            //ToDo
            STScnRangeU* gStart = opq->changes.rngs.arr;
            const ScnSI32 iNxtRng = ScnArraySorted_indexForNew(&opq->changes.rngs, &rng);
            if(iNxtRng < opq->changes.rngs.use && rng.start == gStart[iNxtRng].start && rng.size <= gStart[iNxtRng].size){
                //range already covered by range in current position
            } else if(iNxtRng > 0 && (rng.start + rng.size) <= (gStart[iNxtRng - 1].start + gStart[iNxtRng - 1].size)){
                //range already covered by previous range
            } else {
                //add new range
                ScnArraySorted_addPtr(opq->ctx, &opq->changes.rngs, &rng, STScnRangeU);
            }
        }
        r = ScnTRUE;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}
