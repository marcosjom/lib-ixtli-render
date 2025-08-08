//
//  ScnBuffer.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/scene/ScnBuffer.h"
#include "ixrender/core/ScnArraySorted.h"
#include "ixrender/gpu/ScnGpuBuffer.h"


//STScnBufferChanges

typedef struct STScnBufferChanges {
    ScnContextRef   ctx;
    ScnBOOL         all; //the whole buffer must be synchronized
    ScnUI32         rngsBytesAccum;
    ScnArraySortedStruct(rngs, STScnRangeU);
} STScnBufferChanges;

void    ScnBufferChanges_init(ScnContextRef ctx, STScnBufferChanges* obj);
void    ScnBufferChanges_destroy(STScnBufferChanges* obj);
void    ScnBufferChanges_reset(STScnBufferChanges* obj);
void    ScnBufferChanges_mergeWithOther(STScnBufferChanges* obj, const STScnBufferChanges* const other);
#ifdef SCN_DEBUG
ScnBOOL ScnBufferChanges_validate(STScnBufferChanges* obj);
#endif

SC_INLN ScnBOOL ScnBufferChanges_mergeRng(STScnBufferChanges* obj, const STScnRangeU* rng){
    SCN_ASSERT(!obj->all) //optimization, call only if obj->all == ScnFALSE
    STScnRangeU* gStart = obj->rngs.arr;
    const ScnSI32 iNxtRng = ScnArraySorted_indexForNew(&obj->rngs, rng); SCN_ASSERT(iNxtRng >= 0)
    STScnRangeU* mrged = NULL;
    //eval against prev range
    if(iNxtRng > 0){
        STScnRangeU* prv = &gStart[iNxtRng - 1];
        SCN_ASSERT(prv->start <= rng->start)
        if((prv->start + prv->size) >= (rng->start + rng->size)){
            //new range fully covered by previous range
            return ScnTRUE;
        } else if((prv->start + prv->size) >= rng->start){
            //new range expands prev range
            SCN_ASSERT((rng->start + rng->size) >= prv->start);
            obj->rngsBytesAccum += (rng->start + rng->size) - prv->start - prv->size;
            prv->size = (rng->start + rng->size) - prv->start;
            mrged = prv;
            rng = NULL;
        }
        SCN_ASSERT(rng == NULL || (prv->start + prv->size) < rng->start)
    }
    //eval against next range
    if(mrged == NULL && iNxtRng < obj->rngs.use){
        STScnRangeU* nxt = &gStart[iNxtRng];
        SCN_ASSERT(rng->start <= nxt->start)
        if(nxt->start == rng->start && (nxt->start + nxt->size) >= (rng->start + rng->size)){
            //new range fully covered by next range
            return ScnTRUE;
        } else if((rng->start + rng->size) >= nxt->start){
            //new range expands next range
            SCN_ASSERT((nxt->start + nxt->size) >= rng->start);
            obj->rngsBytesAccum += (nxt->start - rng->start);
            nxt->size = (nxt->start + nxt->size) - rng->start;
            nxt->start = rng->start;
            mrged = nxt;
            rng = NULL;
        }
    }
    //eval new merged-range consuming othe ranges
    if(mrged != NULL){
        STScnRangeU* prev = mrged++;
        const STScnRangeU* gAfterEnd = obj->rngs.arr + obj->rngs.use;
        while(mrged < gAfterEnd){
            SCN_ASSERT(prev->start < mrged->start)
            if((prev->start + prev->size) >= mrged->start){
                //new merged range consumes next range
                if((prev->start + prev->size) < (mrged->start + mrged->size)){
                    //prev range is expanded by next range
                    obj->rngsBytesAccum -= (prev->start + prev->size) - mrged->start;
                    prev->size = (mrged->start + mrged->size) - prev->start;
                }
                //remove
                {
                    STScnRangeU* r = mrged + 1;
                    while(r < gAfterEnd){
                        *(r - 1) = *r;
                        r++;
                    }
                }
                //
                --mrged;
                --gAfterEnd;
                --obj->rngs.use;
            }
            //next
            prev = mrged++;
        }
    }
    SCN_ASSERT(ScnBufferChanges_validate(obj));
    //add new range (not merged)
    if(rng != NULL && NULL != ScnArraySorted_addPtr(obj->ctx, &obj->rngs, rng, STScnRangeU)){
        obj->rngsBytesAccum += rng->size;
        return ScnTRUE;
    }
    return ScnFALSE;
}

SC_INLN void ScnBufferChanges_invalidateAll(STScnBufferChanges* obj){
    obj->all = ScnTRUE;
    ScnArraySorted_empty(&obj->rngs);
}

//STScnBufferSlot

typedef struct STScnBufferSlot {
    ScnContextRef   ctx;
    ScnGpuBufferRef gpuBuff;
    //state
    struct {
        ScnUI32     totalSzLast;
    } state;
    STScnBufferChanges changes;
} STScnBufferSlot;

void ScnBufferSlot_init(ScnContextRef ctx, STScnBufferSlot* opq);
void ScnBufferSlot_destroy(STScnBufferSlot* opq);

//STScnBufferOpq

typedef struct STScnBufferOpq {
    ScnContextRef       ctx;
    ScnMutexRef         mutex;
    //
    STScnGpuBufferCfg   cfg;    //config
    ScnMemElasticRef    mem;    //memory
    ScnGpuDeviceRef     gpuDev;
    //state
    struct {
        ScnUI32         totalSzLast;
    } state;
    STScnBufferChanges  changes;
    //slots (render)
    struct {
        STScnBufferSlot* arr;
        ScnUI16         use;
        ScnUI16         sz;
        ScnUI16         iCur;
    } slots;
} STScnBufferOpq;

ScnSI32 ScnBuffer_getOpqSz(void){
    return (ScnSI32)sizeof(STScnBufferOpq);
}

void ScnBuffer_initZeroedOpq(ScnContextRef ctx, void* obj) {
    STScnBufferOpq* opq = (STScnBufferOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex = ScnContext_allocMutex(opq->ctx);
    //
    ScnBufferChanges_init(opq->ctx, &opq->changes);
}

void ScnBuffer_destroyOpq(void* obj){
    STScnBufferOpq* opq = (STScnBufferOpq*)obj;
    //
    ScnMemElastic_releaseAndNull(&opq->mem);
    //ScnStruct_stRelease(ScnBufferCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
    ScnBufferChanges_destroy(&opq->changes);
    //slots
    {
        if(opq->slots.arr != NULL){
            STScnBufferSlot* s = opq->slots.arr;
            const STScnBufferSlot* sAfterEnd = s + opq->slots.use;
            while(s < sAfterEnd){
                ScnBufferSlot_destroy(s);
                ++s;
            }
            ScnContext_mfree(opq->ctx, opq->slots.arr);
            opq->slots.arr = NULL;
        }
        opq->slots.use = opq->slots.sz = 0;
    }
    ScnGpuDevice_releaseAndNull(&opq->gpuDev);
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNull(&opq->ctx);
}

//

ScnBOOL ScnBuffer_prepare(ScnBufferRef ref, ScnGpuDeviceRef gpuDev, const ScnUI32 ammRenderSlots, const STScnGpuBufferCfg* const cfg) {
    ScnBOOL r = ScnFALSE;
    STScnBufferOpq* opq = (STScnBufferOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(cfg != NULL && ammRenderSlots > 0 && ScnMemElastic_isNull(opq->mem) && opq->slots.arr == NULL && !ScnGpuDevice_isNull(gpuDev)){
        ScnUI32 totalSz = 0;
        ScnMemElasticRef mem = ScnMemElastic_alloc(opq->ctx);
        if(ScnMemElastic_prepare(mem, &cfg->mem, &totalSz)){
            STScnBufferSlot* slots = ScnContext_malloc(opq->ctx, sizeof(STScnBufferSlot) * ammRenderSlots, "ScnBuffer_prepare::slots");
            if(slots != NULL){
                //slots (render)
                {
                    STScnBufferSlot* s = slots;
                    const STScnBufferSlot* sAfterEnd = s + ammRenderSlots;
                    while(s < sAfterEnd){
                        ScnBufferSlot_init(opq->ctx, s);
                        //force first full-sync
                        ScnBufferChanges_invalidateAll(&s->changes);
                        //
                        ++s;
                    }
                    opq->slots.arr = slots;
                    opq->slots.use = opq->slots.sz = ammRenderSlots;
                }
                //set
                ScnMemElastic_set(&opq->mem, mem);
                ScnGpuDevice_set(&opq->gpuDev, gpuDev);
                //
                opq->cfg = *cfg;
                //ScnStruct_stRelease(ScnBufferCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
                //ScnStruct_stClone(ScnBufferCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
                //state
                {
                    opq->state.totalSzLast = totalSz;
                    
                }
                r = ScnTRUE;
            }
        }
        //relese (if not consumed)
        ScnMemElastic_releaseAndNull(&mem);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnBuffer_hasPtrs(ScnBufferRef ref){ //allocations made?
    ScnBOOL r = ScnFALSE;
    STScnBufferOpq* opq = (STScnBufferOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnMemElastic_isNull(opq->mem)){
        r = ScnMemElastic_hasPtrs(opq->mem);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnUI32 ScnBuffer_getRenderSlotsCount(ScnBufferRef ref){
    STScnBufferOpq* opq = (STScnBufferOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->slots.use;
}

//cpu-buffer

ScnBOOL ScnBuffer_clear(ScnBufferRef ref){
    ScnBOOL r = ScnFALSE;
    STScnBufferOpq* opq = (STScnBufferOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnMemElastic_isNull(opq->mem)){
        ScnMemElastic_clear(opq->mem);
        r = ScnTRUE;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnBuffer_invalidateAll(ScnBufferRef ref){ //forces the full buffer to be synced to its gpu-buffer slot
    ScnBOOL r = ScnFALSE;
    STScnBufferOpq* opq = (STScnBufferOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnMemElastic_isNull(opq->mem)){
        //changes
        ScnBufferChanges_invalidateAll(&opq->changes);
        r = ScnTRUE;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnBuffer_mInvalidateLockedOpq_(STScnBufferOpq* opq, const STScnAbsPtr ptr, const ScnUI32 sz){
    ScnBOOL r = ScnTRUE;
    STScnRangeU rng = STScnRangeU_Zero;
    rng.start   = ptr.idx / opq->cfg.mem.sizeAlign * opq->cfg.mem.sizeAlign;
    rng.size    = ((ptr.idx + sz + opq->cfg.mem.sizeAlign - 1) / opq->cfg.mem.sizeAlign * opq->cfg.mem.sizeAlign) - rng.start;
    SCN_ASSERT(rng.start <= ptr.idx && (ptr.idx + sz) <= (rng.start + rng.size))
    if(!ScnBufferChanges_mergeRng(&opq->changes, &rng)){
        r = ScnFALSE;
    }
    SCN_ASSERT(ScnBufferChanges_validate(&opq->changes))
    return r;
}
    
STScnAbsPtr ScnBuffer_malloc(ScnBufferRef ref, const ScnUI32 usableSz){
    STScnAbsPtr r = STScnAbsPtr_Zero;
    STScnBufferOpq* opq = (STScnBufferOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnMemElastic_isNull(opq->mem)){
        ScnUI32 totalSz = 0;
        r = ScnMemElastic_malloc(opq->mem, usableSz, &totalSz);
        if(opq->state.totalSzLast != totalSz){
            opq->state.totalSzLast = totalSz;
            ScnBufferChanges_invalidateAll(&opq->changes);
        } else if(r.ptr != NULL && usableSz > 0 && !opq->changes.all){
            ScnBuffer_mInvalidateLockedOpq_(opq, r, usableSz);
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnBuffer_mfree(ScnBufferRef ref, const STScnAbsPtr ptr){
    ScnBOOL r = ScnFALSE;
    STScnBufferOpq* opq = (STScnBufferOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnMemElastic_isNull(opq->mem)){
        r = ScnMemElastic_mfree(opq->mem, ptr);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnBuffer_mInvalidate(ScnBufferRef ref, const STScnAbsPtr ptr, const ScnUI32 sz){
    ScnBOOL r = ScnFALSE;
    STScnBufferOpq* opq = (STScnBufferOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnMemElastic_isNull(opq->mem) && opq->cfg.mem.sizeAlign > 0){
        r = ScnTRUE;
        if(sz > 0 && !opq->changes.all){ //no need to evaluate rngs if the whole buffer requires an update
            r = ScnBuffer_mInvalidateLockedOpq_(opq, ptr, sz);
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//gpu-buffer

ScnBOOL ScnBuffer_prepareCurrentRenderSlot(ScnBufferRef ref, ScnBOOL* dstHasPtrs){
    ScnBOOL r = ScnFALSE; ScnBOOL hasPtrs = ScnFALSE;
    STScnBufferOpq* opq = (STScnBufferOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->slots.arr != NULL && opq->slots.use > 0 && !ScnMemElastic_isNull(opq->mem) && opq->cfg.mem.sizeAlign > 0 && !ScnGpuDevice_isNull(opq->gpuDev)){
        //sync gpu-buffer
        const ScnUI32 iFirst = opq->slots.iCur % opq->slots.use;
        STScnBufferSlot* slot = &opq->slots.arr[iFirst];
        //forward accumulated changes to all slots
        {
            ScnUI32 iSlot = iFirst;
            do {
                STScnBufferSlot* slot = &opq->slots.arr[iSlot];
                ScnBufferChanges_mergeWithOther(&slot->changes, &opq->changes);
                //next
                iSlot = (iSlot + 1) % opq->slots.use;
            } while(iSlot != iFirst);
            //reset accumulates changes
            ScnBufferChanges_reset(&opq->changes);
        }
        //
        hasPtrs = ScnMemElastic_hasPtrs(opq->mem);
        if(!hasPtrs){
            //nothing to sync
            r = ScnTRUE;
        } else if(ScnGpuBuffer_isNull(slot->gpuBuff)){
            slot->gpuBuff = ScnGpuDevice_allocBuffer(opq->gpuDev, opq->mem);
            if(!ScnGpuBuffer_isNull(slot->gpuBuff)){
                r = ScnTRUE;
            }
        } else {
            STScnGpuBufferChanges changes = STScnGpuBufferChanges_Zero;
            changes.all = slot->changes.all;
            if(!changes.all){
                changes.rngs = slot->changes.rngs.arr;
                changes.rngsUse = slot->changes.rngs.use;
            }
            if(!changes.all && changes.rngsUse == 0){
                //no changes to update
                r = ScnTRUE;
            } else {
                r = ScnGpuBuffer_sync(slot->gpuBuff, &opq->cfg, opq->mem, &changes);
            }
        }
        //reset current slot changes
        ScnBufferChanges_reset(&slot->changes);
    }
    ScnMutex_unlock(opq->mutex);
    //
    if(dstHasPtrs != NULL) *dstHasPtrs = hasPtrs;
    //
    return r;
}

ScnBOOL ScnBuffer_moveToNextRenderSlot(ScnBufferRef ref){
    ScnBOOL r = ScnFALSE;
    STScnBufferOpq* opq = (STScnBufferOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->slots.arr != NULL && opq->slots.use > 0 && !ScnMemElastic_isNull(opq->mem) && opq->cfg.mem.sizeAlign > 0 && !ScnGpuDevice_isNull(opq->gpuDev)){
        //move to next render slot
        opq->slots.iCur = (opq->slots.iCur + 1) % opq->slots.use;
        r = ScnTRUE;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnGpuBufferRef ScnBuffer_getCurrentRenderSlot(ScnBufferRef ref){
    ScnGpuBufferRef r = ScnObjRef_Zero;
    STScnBufferOpq* opq = (STScnBufferOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->slots.arr != NULL && opq->slots.use > 0){
        STScnBufferSlot* slot = &opq->slots.arr[opq->slots.iCur % opq->slots.use];
        r = slot->gpuBuff;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//STScnBufferChanges

void ScnBufferChanges_init(ScnContextRef ctx, STScnBufferChanges* obj){
    memset(obj, 0, sizeof(*obj));
    ScnContext_set(&obj->ctx, ctx);
    ScnArraySorted_init(ctx, &obj->rngs, 0, 128, STScnRangeU, ScnCompare_STScnRangeU);
}

void ScnBufferChanges_destroy(STScnBufferChanges* obj){
    ScnArraySorted_destroy(obj->ctx, &obj->rngs);
    ScnContext_releaseAndNull(&obj->ctx);
}

#ifdef SCN_DEBUG
ScnBOOL ScnBufferChanges_validate(STScnBufferChanges* obj){
    ScnBOOL r = ScnTRUE;
    ScnUI32 prevAfterEnd = 0, szAccum = 0;
    const STScnRangeU* rng = obj->rngs.arr;
    const STScnRangeU* rngAfterEnd = rng + obj->rngs.use;
    while(rng < rngAfterEnd){
        if(prevAfterEnd > rng->start){
            SCN_ASSERT(ScnFALSE)
            r = ScnFALSE;
            break;
        }
        prevAfterEnd = rng->start + rng->size;
        szAccum += rng->size;
        //next
        ++rng;
    }
    if(obj->rngsBytesAccum != szAccum){
        SCN_ASSERT(ScnFALSE)
        r = ScnFALSE;
    }
    return r;
}
#endif

void ScnBufferChanges_reset(STScnBufferChanges* obj){
    obj->all = ScnFALSE;
    obj->rngsBytesAccum = 0;
    ScnArraySorted_empty(&obj->rngs);
}

void ScnBufferChanges_mergeWithOther(STScnBufferChanges* obj, const STScnBufferChanges* const other){
    if(!obj->all){
        if(other->all){
            //all
            ScnBufferChanges_reset(obj);
            obj->all = ScnTRUE;
        } else {
            //merge ranges
            const STScnRangeU* rng = other->rngs.arr;
            if(obj->rngs.use == 0){
                //memcpy
                if(other->rngs.use > 0){
                    ScnArraySorted_prepareForGrowth(obj->ctx, &obj->rngs, other->rngs.use, STScnRangeU);
                    if(other->rngs.use <= obj->rngs.sz){
                        memcpy(obj->rngs.arr, other->rngs.arr, sizeof(obj->rngs.arr[0]) * other->rngs.use);
                        obj->rngs.use = other->rngs.use;
                    }
                }
                obj->rngsBytesAccum = other->rngsBytesAccum;
                SCN_ASSERT(obj->rngs.use == other->rngs.use)
                SCN_ASSERT(ScnBufferChanges_validate(obj))
            } else {
                //one by one
                const STScnRangeU* rngAfterEnd = rng + other->rngs.use;
                while(rng < rngAfterEnd){
                    if(!ScnBufferChanges_mergeRng(obj, rng)){
                        break;
                    }
                    //next
                    ++rng;
                }
                SCN_ASSERT(ScnBufferChanges_validate(obj))
            }
        }
    }
}

//STScnBufferSlot

void ScnBufferSlot_init(ScnContextRef ctx, STScnBufferSlot* opq){
    memset(opq, 0, sizeof(*opq));
    ScnContext_set(&opq->ctx, ctx);
    ScnBufferChanges_init(opq->ctx, &opq->changes);
}

void ScnBufferSlot_destroy(STScnBufferSlot* opq){
    ScnBufferChanges_destroy(&opq->changes);
    ScnGpuBuffer_releaseAndNull(&opq->gpuBuff);
    ScnContext_releaseAndNull(&opq->ctx);
}
