//
//  ScnGpuDevice.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/gpu/ScnGpuDevice.h"

//STScnGpuDeviceOpq

typedef struct STScnGpuDeviceOpq_ {
    STScnContextRef     ctx;
    STScnMutexRef       mutex;
    //
    STScnGpuDeviceCfg cfg;    //config
    //api
    struct {
        STScnGpuDeviceApiItf itf;
        void*           itfParam;
        void*           data;
    } api;
} STScnGpuDeviceOpq;

ScnSI32 ScnGpuDevice_getOpqSz(void){
    return (ScnSI32)sizeof(STScnGpuDeviceOpq);
}

void ScnGpuDevice_initZeroedOpq(STScnContextRef ctx, void* obj) {
    STScnGpuDeviceOpq* opq = (STScnGpuDeviceOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex = ScnContext_mutex_alloc(opq->ctx);
}

void ScnGpuDevice_destroyOpq(void* obj){
    STScnGpuDeviceOpq* opq = (STScnGpuDeviceOpq*)obj;
    //api
    {
        if(opq->api.data != NULL){
            if(opq->api.itf.free != NULL){
                (*opq->api.itf.free)(opq->api.data, opq->api.itfParam);
            }
            opq->api.data = NULL;
        }
        ScnMemory_setZeroSt(opq->api.itf, STScnGpuDeviceApiItf);
        opq->api.itfParam = NULL;
    }
    //
    //ScnStruct_stRelease(ScnGpuDeviceCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
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

ScnBOOL ScnGpuDevice_prepare(STScnGpuDeviceRef ref, const STScnGpuDeviceCfg* cfg, const STScnGpuDeviceApiItf* itf, void* itfParam) {
    ScnBOOL r = ScnFALSE;
    STScnGpuDeviceOpq* opq = (STScnGpuDeviceOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(cfg != NULL && itf != NULL && itf->alloc != NULL && itf->free != NULL){
        void* data = (*itf->alloc)(opq->ctx, cfg, itfParam);
        if(data != NULL){
            opq->cfg = *cfg;
            //ScnStruct_stRelease(ScnGpuDeviceCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
            //ScnStruct_stClone(ScnGpuDeviceCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
            //api
            {
                if(opq->api.data != NULL){
                    if(opq->api.itf.free != NULL){
                        (*opq->api.itf.free)(opq->api.data, opq->api.itfParam);
                    }
                    opq->api.data = NULL;
                }
                ScnMemory_setZeroSt(opq->api.itf, STScnGpuDeviceApiItf);
                opq->api.itfParam = NULL;
                //
                if(itf != NULL){
                    opq->api.itf = *itf;
                    opq->api.itfParam = itfParam;
                }
                //data
                opq->api.data = data; data = NULL; //consume
            }
        }
        //destroy (if not consumed)
        if(data != NULL && itf->free != NULL){
            (*itf->free)(data, itfParam);
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}
