//
//  ScnGpuDevice.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/gpu/ScnGpuDevice.h"

//STScnGpuDeviceOpq

typedef struct STScnGpuDeviceOpq_ {
    //api
    struct {
        STScnGpuDeviceApiItf itf;
        void*           itfParam;
    } api;
} STScnGpuDeviceOpq;

ScnSI32 ScnGpuDevice_getOpqSz(void){
    return (ScnSI32)sizeof(STScnGpuDeviceOpq);
}

void ScnGpuDevice_initZeroedOpq(STScnContextRef ctx, void* obj) {
    //STScnGpuDeviceOpq* opq = (STScnGpuDeviceOpq*)obj;
}

void ScnGpuDevice_destroyOpq(void* obj){
    STScnGpuDeviceOpq* opq = (STScnGpuDeviceOpq*)obj;
    //api
    {
        if(opq->api.itf.free != NULL){
            (*opq->api.itf.free)(opq->api.itfParam);
        }
        ScnMemory_setZeroSt(opq->api.itf, STScnGpuDeviceApiItf);
        opq->api.itfParam = NULL;
    }
}

//

ScnBOOL ScnGpuDevice_prepare(STScnGpuDeviceRef ref, const STScnGpuDeviceApiItf* itf, void* itfParam) {
    ScnBOOL r = ScnFALSE;
    STScnGpuDeviceOpq* opq = (STScnGpuDeviceOpq*)ScnSharedPtr_getOpq(ref.ptr);
    if(itf != NULL && itf->free != NULL){
        //api
        {
            if(opq->api.itf.free != NULL){
                (*opq->api.itf.free)(opq->api.itfParam);
            }
            ScnMemory_setZeroSt(opq->api.itf, STScnGpuDeviceApiItf);
            opq->api.itfParam = NULL;
            //
            if(itf != NULL){
                opq->api.itf = *itf;
                opq->api.itfParam = itfParam;
            }
            r = ScnTRUE;
        }
    }
    return r;
}

STScnGpuBufferRef ScnGpuDevice_allocBuffer(STScnGpuDeviceRef ref, STScnMemElasticRef mem){
    STScnGpuDeviceOpq* opq = (STScnGpuDeviceOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq->api.itf.allocBuffer != NULL ? (*opq->api.itf.allocBuffer)(opq->api.itfParam, mem) : (STScnGpuBufferRef)STScnObjRef_Zero);
}

STScnGpuVertexbuffRef ScnGpuDevice_allocVertexBuff(STScnGpuDeviceRef ref, const STScnGpuVertexbuffCfg* cfg, STScnGpuBufferRef vBuff, STScnGpuBufferRef idxBuff){
    STScnGpuDeviceOpq* opq = (STScnGpuDeviceOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq->api.itf.allocVertexBuff != NULL ? (*opq->api.itf.allocVertexBuff)(opq->api.itfParam, cfg, vBuff, idxBuff) : (STScnGpuVertexbuffRef)STScnObjRef_Zero);
}
