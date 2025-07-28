//
//  ScnGpuVertexbuff.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/gpu/ScnGpuVertexbuff.h"
#include "ixrender/gpu/ScnGpuBuffer.h"

//STScnGpuVertexbuffOpq

typedef struct STScnGpuVertexbuffOpq_ {
    STScnContextRef     ctx;
    STScnMutexRef       mutex;
    //
    STScnGpuVertexbuffCfg    cfg;    //config
    //buffs
    struct {
        STScnGpuBufferRef vertex;
        STScnGpuBufferRef idxs;
    } buffs;
    //api
    struct {
        STScnGpuVertexbuffApiItf itf;
        void*               itfParam;
        void*               data;
    } api;
} STScnGpuVertexbuffOpq;

ScnSI32 ScnGpuVertexbuff_getOpqSz(void){
    return (ScnSI32)sizeof(STScnGpuVertexbuffOpq);
}

void ScnGpuVertexbuff_initZeroedOpq(STScnContextRef ctx, void* obj) {
    STScnGpuVertexbuffOpq* opq = (STScnGpuVertexbuffOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex = ScnContext_mutex_alloc(opq->ctx);

}

void ScnGpuVertexbuff_destroyOpq(void* obj){
    STScnGpuVertexbuffOpq* opq = (STScnGpuVertexbuffOpq*)obj;
    //api
    {
        if(opq->api.data != NULL){
            if(opq->api.itf.destroy != NULL){
                (*opq->api.itf.destroy)(opq->api.data, opq->api.itfParam);
            }
            opq->api.data = NULL;
        }
        ScnMemory_setZeroSt(opq->api.itf, STScnGpuVertexbuffApiItf);
        opq->api.itfParam = NULL;
    }
    //buffs
    {
        if(!ScnGpuBuffer_isNull(opq->buffs.vertex)){
            ScnGpuBuffer_release(&opq->buffs.vertex);
            ScnGpuBuffer_null(&opq->buffs.vertex);
        }
        if(!ScnGpuBuffer_isNull(opq->buffs.idxs)){
            ScnGpuBuffer_release(&opq->buffs.idxs);
            ScnGpuBuffer_null(&opq->buffs.idxs);
        }
    }
    //
    //ScnStruct_stRelease(ScnGpuVertexbuffCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
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

ScnBOOL ScnGpuVertexbuff_prepare(STScnGpuVertexbuffRef ref, const STScnGpuVertexbuffCfg* cfg, STScnGpuBufferRef vertexBuff, STScnGpuBufferRef idxsBuff, const STScnGpuVertexbuffApiItf* itf, void* itfParam) {
    ScnBOOL r = ScnFALSE;
    STScnGpuVertexbuffOpq* opq = (STScnGpuVertexbuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(cfg != NULL && itf != NULL && itf->create != NULL && itf->destroy != NULL && !ScnGpuBuffer_isNull(vertexBuff)){
        void* data = (*itf->create)(cfg, vertexBuff, idxsBuff, itfParam);
        if(data != NULL){
            //
            opq->cfg = *cfg;
            //ScnStruct_stRelease(ScnGpuVertexbuffCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
            //ScnStruct_stClone(ScnGpuVertexbuffCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
            //api
            {
                if(opq->api.data != NULL){
                    if(opq->api.itf.destroy != NULL){
                        (*opq->api.itf.destroy)(opq->api.data, opq->api.itfParam);
                    }
                    opq->api.data = NULL;
                }
                if(!ScnGpuBuffer_isNull(opq->buffs.vertex)){
                    ScnGpuBuffer_release(&opq->buffs.vertex);
                    ScnGpuBuffer_null(&opq->buffs.vertex);
                }
                if(!ScnGpuBuffer_isNull(opq->buffs.idxs)){
                    ScnGpuBuffer_release(&opq->buffs.idxs);
                    ScnGpuBuffer_null(&opq->buffs.idxs);
                }
                //
                ScnMemory_setZeroSt(opq->api.itf, STScnGpuVertexbuffApiItf);
                opq->api.itfParam = NULL;
                //
                if(itf != NULL){
                    opq->api.itf = *itf;
                    opq->api.itfParam = itfParam;
                }
                //data
                opq->api.data = data; data = NULL; //consume
                //
                ScnGpuBuffer_set(&opq->buffs.vertex, vertexBuff);
                ScnGpuBuffer_set(&opq->buffs.idxs, idxsBuff);
            }
            r = ScnTRUE;
        }
        //destroy (if not consumed)
        if(data != NULL && itf->destroy != NULL){
            (*itf->destroy)(data, itfParam);
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnGpuVertexbuff_activate(STScnGpuVertexbuffRef ref){
    ScnBOOL r = ScnFALSE;
    STScnGpuVertexbuffOpq* opq = (STScnGpuVertexbuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->api.itf.activate != NULL){
        r = (*opq->api.itf.activate)(opq->api.data, &opq->cfg, opq->api.itfParam);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnGpuVertexbuff_deactivate(STScnGpuVertexbuffRef ref){
    ScnBOOL r = ScnFALSE;
    STScnGpuVertexbuffOpq* opq = (STScnGpuVertexbuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->api.itf.deactivate != NULL){
        r = (*opq->api.itf.deactivate)(opq->api.data, opq->api.itfParam);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//

ScnUI32 ScnGpuVertexbuff_getSzPerRecord(STScnGpuVertexbuffRef ref){
    STScnGpuVertexbuffOpq* opq = (STScnGpuVertexbuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->cfg.szPerRecord;
}

STScnGpuBufferRef ScnGpuVertexbuff_getVertexBuff(STScnGpuVertexbuffRef ref){
    STScnGpuVertexbuffOpq* opq = (STScnGpuVertexbuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->buffs.vertex;
}

STScnGpuBufferRef ScnGpuVertexbuff_getIdxsBuff(STScnGpuVertexbuffRef ref){
    STScnGpuVertexbuffOpq* opq = (STScnGpuVertexbuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->buffs.idxs;
}
