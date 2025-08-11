//
//  ScnBuffer.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/scene/ScnBuffer.h"
#include "ixrender/core/ScnArraySorted.h"
#include "ixrender/type/ScnChangeRngs.h"
#include "ixrender/gpu/ScnGpuBuffer.h"

//STScnBufferSlot

typedef struct STScnBufferSlot {
    ScnContextRef   ctx;
    ScnGpuBufferRef gpuBuff;
    //state
    struct {
        ScnUI32     totalSzLast;
    } state;
    STScnChangesRngs changes;
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
    STScnChangesRngs  changes;
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
    ScnChangesRngs_init(opq->ctx, &opq->changes);
}

void ScnBuffer_destroyOpq(void* obj){
    STScnBufferOpq* opq = (STScnBufferOpq*)obj;
    //
    ScnMemElastic_releaseAndNull(&opq->mem);
    //ScnStruct_stRelease(ScnBufferCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
    ScnChangesRngs_destroy(&opq->changes);
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
                        ScnChangesRngs_invalidateAll(&s->changes);
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
        ScnChangesRngs_invalidateAll(&opq->changes);
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
    if(!ScnChangesRngs_mergeRng(&opq->changes, &rng)){
        r = ScnFALSE;
    }
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
            ScnChangesRngs_invalidateAll(&opq->changes);
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
                ScnChangesRngs_mergeWithOther(&slot->changes, &opq->changes);
                //next
                iSlot = (iSlot + 1) % opq->slots.use;
            } while(iSlot != iFirst);
            //reset accumulates changes
            ScnChangesRngs_reset(&opq->changes);
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
            changes.all = slot->changes.all; //ToDo: disable this
            if(!changes.all){
                changes.rngs    = slot->changes.rngs.arr;
                changes.rngsUse = slot->changes.rngs.use;
            }
            if(!changes.all && changes.rngsUse == 0){
                //no changes to update
                r = ScnTRUE;
            } else {
                r = ScnGpuBuffer_sync(slot->gpuBuff, opq->mem, &changes);
            }
        }
        //reset current slot changes
        ScnChangesRngs_reset(&slot->changes);
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

//STScnBufferSlot

void ScnBufferSlot_init(ScnContextRef ctx, STScnBufferSlot* opq){
    ScnMemory_setZeroSt(*opq);
    ScnContext_set(&opq->ctx, ctx);
    ScnChangesRngs_init(opq->ctx, &opq->changes);
}

void ScnBufferSlot_destroy(STScnBufferSlot* opq){
    ScnChangesRngs_destroy(&opq->changes);
    ScnGpuBuffer_releaseAndNull(&opq->gpuBuff);
    ScnContext_releaseAndNull(&opq->ctx);
}
