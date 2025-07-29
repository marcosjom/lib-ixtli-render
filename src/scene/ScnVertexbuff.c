//
//  ScnVertexbuff.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/scene/ScnVertexbuff.h"
#include "ixrender/scene/ScnBuffer.h"

//STScnVertexbuffSlot

typedef struct STScnVertexbuffSlot_ {
    STScnGpuVertexbuffRef   gpuVBuff;
} STScnVertexbuffSlot;

void ScnVertexbuffSlot_init(STScnContextRef ctx, STScnVertexbuffSlot* opq);
void ScnVertexbuffSlot_destroy(STScnVertexbuffSlot* opq);

//STScnVertexbuffOpq

typedef struct STScnVertexbuffOpq_ {
    STScnContextRef         ctx;
    STScnMutexRef           mutex;
    //
    STScnGpuVertexbuffCfg   cfg;    //config
    STScnGpuDeviceRef       gpuDev;
    //buffs
    struct {
        STScnBufferRef      vertex;
        STScnBufferRef      idxs;
    } buffs;
    //slots (render)
    struct {
        STScnVertexbuffSlot* arr;
        ScnUI16             use;
        ScnUI16             sz;
        ScnUI16             iCur;
    } slots;
} STScnVertexbuffOpq;

ScnSI32 ScnVertexbuff_getOpqSz(void){
    return (ScnSI32)sizeof(STScnVertexbuffOpq);
}

void ScnVertexbuff_initZeroedOpq(STScnContextRef ctx, void* obj) {
    STScnVertexbuffOpq* opq = (STScnVertexbuffOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex = ScnContext_mutex_alloc(opq->ctx);

}

void ScnVertexbuff_destroyOpq(void* obj){
    STScnVertexbuffOpq* opq = (STScnVertexbuffOpq*)obj;
    //buffs
    {
        if(!ScnBuffer_isNull(opq->buffs.vertex)){
            ScnBuffer_release(&opq->buffs.vertex);
            ScnBuffer_null(&opq->buffs.vertex);
        }
        if(!ScnBuffer_isNull(opq->buffs.idxs)){
            ScnBuffer_release(&opq->buffs.idxs);
            ScnBuffer_null(&opq->buffs.idxs);
        }
    }
    //
    //ScnStruct_stRelease(ScnVertexbuffCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
    //
    //gpuDev
    if(!ScnGpuDevice_isNull(opq->gpuDev)){
        ScnGpuDevice_release(&opq->gpuDev);
        ScnGpuDevice_null(&opq->gpuDev);
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

ScnBOOL ScnVertexbuff_prepare(STScnVertexbuffRef ref, STScnGpuDeviceRef gpuDev, const ScnUI32 ammRenderSlots, const STScnGpuVertexbuffCfg* cfg, STScnBufferRef vertexBuff, STScnBufferRef idxsBuff) {
    ScnBOOL r = ScnFALSE;
    STScnVertexbuffOpq* opq = (STScnVertexbuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    SCN_ASSERT(ScnBuffer_getRenderSlotsCount(vertexBuff) == ScnBuffer_getRenderSlotsCount(idxsBuff))
    SCN_ASSERT(ScnBuffer_getRenderSlotsCount(vertexBuff) == ammRenderSlots)
    if(!ScnGpuDevice_isNull(gpuDev) && cfg != NULL && ammRenderSlots > 0 && !ScnBuffer_isNull(vertexBuff) && opq->slots.arr == NULL
       && ScnBuffer_getRenderSlotsCount(vertexBuff) == ScnBuffer_getRenderSlotsCount(idxsBuff)
       && ScnBuffer_getRenderSlotsCount(vertexBuff) == ammRenderSlots
       )
    {
        STScnVertexbuffSlot* slots = ScnContext_malloc(opq->ctx, sizeof(STScnVertexbuffSlot) * ammRenderSlots, "ScnBuffer_prepare::slots");
        if(slots != NULL){
            //slots (render)
            {
                STScnVertexbuffSlot* s = slots;
                const STScnVertexbuffSlot* sAfterEnd = s + ammRenderSlots;
                while(s < sAfterEnd){
                    ScnVertexbuffSlot_init(opq->ctx, s);
                    ++s;
                }
                opq->slots.arr = slots;
                opq->slots.use = opq->slots.sz = ammRenderSlots;
            }
            opq->cfg = *cfg;
            ScnGpuDevice_set(&opq->gpuDev, gpuDev);
            //ScnStruct_stRelease(ScnVertexbuffCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
            //ScnStruct_stClone(ScnVertexbuffCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
            //
            ScnBuffer_set(&opq->buffs.vertex, vertexBuff);
            ScnBuffer_set(&opq->buffs.idxs, idxsBuff);
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//

ScnUI32 ScnVertexbuff_getSzPerRecord(STScnVertexbuffRef ref){
    STScnVertexbuffOpq* opq = (STScnVertexbuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->cfg.szPerRecord;
}

STScnBufferRef ScnVertexbuff_getVertexBuff(STScnVertexbuffRef ref){
    STScnVertexbuffOpq* opq = (STScnVertexbuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->buffs.vertex;
}

STScnBufferRef ScnVertexbuff_getIdxsBuff(STScnVertexbuffRef ref){
    STScnVertexbuffOpq* opq = (STScnVertexbuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->buffs.idxs;
}

//STScnVertexbuffSlot

void ScnVertexbuffSlot_init(STScnContextRef ctx, STScnVertexbuffSlot* opq){
    memset(opq, 0, sizeof(*opq));
}

void ScnVertexbuffSlot_destroy(STScnVertexbuffSlot* opq){
    if(!ScnGpuVertexbuff_isNull(opq->gpuVBuff)){
        ScnGpuVertexbuff_release(&opq->gpuVBuff);
        ScnGpuVertexbuff_null(&opq->gpuVBuff);
    }
}

//gpu-vertexbuffer

ScnBOOL ScnVertexbuff_prepareNextRenderSlot(STScnVertexbuffRef ref){
    ScnBOOL r = ScnFALSE;
    STScnVertexbuffOpq* opq = (STScnVertexbuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->slots.arr != NULL && opq->slots.use > 0 && !ScnGpuDevice_isNull(opq->gpuDev)){
        r = ScnTRUE;
        STScnGpuBufferRef vbuff = STScnObjRef_Zero; ScnBOOL vbuffHasPtrs = ScnFALSE;
        STScnGpuBufferRef iBuff = STScnObjRef_Zero; ScnBOOL iBuffHasPtrs = ScnFALSE;
        //move to next render slot
        opq->slots.iCur = (opq->slots.iCur + 1) % opq->slots.use;
        //buffs
        {
            if(r && !ScnBuffer_isNull(opq->buffs.vertex) && !ScnBuffer_prepareNextRenderSlot(opq->buffs.vertex, &vbuffHasPtrs)){
                printf("ERROR ScnVertexbuff_prepareNextRenderSlot::ScnBuffer_prepareNextRenderSlot(vertex) failed.\n");
                r = ScnFALSE;
            } else if(vbuffHasPtrs){
                vbuff = ScnBuffer_getCurrentRenderSlotGpuBuffer(opq->buffs.vertex);
            }
            if(r && !ScnBuffer_isNull(opq->buffs.idxs) && !ScnBuffer_prepareNextRenderSlot(opq->buffs.idxs, &iBuffHasPtrs)){
                printf("ERROR ScnVertexbuff_prepareNextRenderSlot::ScnBuffer_prepareNextRenderSlot(idxs) failed.\n");
                r = ScnFALSE;
            } else if(iBuffHasPtrs){
                iBuff = ScnBuffer_getCurrentRenderSlotGpuBuffer(opq->buffs.idxs);
            }
        }
        //sync gpu-buffer
        if(r && ScnGpuBuffer_isNull(vbuff) && vbuffHasPtrs){
            printf("ERROR ScnVertexbuff_prepareNextRenderSlot::vbuff is NULL.\n");
            r = ScnFALSE;
        } else if(r && ScnGpuBuffer_isNull(iBuff) && iBuffHasPtrs){
            printf("ERROR ScnVertexbuff_prepareNextRenderSlot::iBuff is NULL.\n");
            r = ScnFALSE;
        } else if(!vbuffHasPtrs && !iBuffHasPtrs){
            //nothing to sync
        } else {
            STScnVertexbuffSlot* slot = &opq->slots.arr[opq->slots.iCur];
            if(ScnGpuVertexbuff_isNull(slot->gpuVBuff)){
                slot->gpuVBuff = ScnGpuDevice_allocVertexBuff(opq->gpuDev, &opq->cfg, vbuff, iBuff);
                if(!ScnGpuDevice_isNull(slot->gpuVBuff)){
                    r = ScnTRUE;
                }
            } else {
                r = ScnGpuVertexbuff_sync(slot->gpuVBuff, &opq->cfg, vbuff, iBuff);
            }
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}
