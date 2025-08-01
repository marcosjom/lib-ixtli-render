//
//  ScnGpuDevice.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/gpu/ScnGpuDevice.h"

//STScnGpuDeviceOpq

typedef struct STScnGpuDeviceOpq {
    //api
    struct {
        STScnGpuDeviceApiItf itf;
        void*           itfParam;
    } api;
} STScnGpuDeviceOpq;

ScnSI32 ScnGpuDevice_getOpqSz(void){
    return (ScnSI32)sizeof(STScnGpuDeviceOpq);
}

void ScnGpuDevice_initZeroedOpq(ScnContextRef ctx, void* obj) {
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

ScnBOOL ScnGpuDevice_prepare(ScnGpuDeviceRef ref, const STScnGpuDeviceApiItf* itf, void* itfParam) {
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

void* ScnGpuDevice_getApiDevice(ScnGpuDeviceRef ref){
    STScnGpuDeviceOpq* opq = (STScnGpuDeviceOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq->api.itf.getApiDevice != NULL ? (*opq->api.itf.getApiDevice)(opq->api.itfParam) : NULL);
}

ScnGpuBufferRef ScnGpuDevice_allocBuffer(ScnGpuDeviceRef ref, ScnMemElasticRef mem){
    STScnGpuDeviceOpq* opq = (STScnGpuDeviceOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq->api.itf.allocBuffer != NULL ? (*opq->api.itf.allocBuffer)(opq->api.itfParam, mem) : (ScnGpuBufferRef)ScnObjRef_Zero);
}

ScnGpuVertexbuffRef ScnGpuDevice_allocVertexBuff(ScnGpuDeviceRef ref, const STScnGpuVertexbuffCfg* cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff){
    STScnGpuDeviceOpq* opq = (STScnGpuDeviceOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq->api.itf.allocVertexBuff != NULL ? (*opq->api.itf.allocVertexBuff)(opq->api.itfParam, cfg, vBuff, idxBuff) : (ScnGpuVertexbuffRef)ScnObjRef_Zero);
}

ScnGpuFramebuffRef ScnGpuDevice_allocFramebuffFromOSView(ScnGpuDeviceRef ref, void* mtkView){
    STScnGpuDeviceOpq* opq = (STScnGpuDeviceOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq->api.itf.allocFramebuffFromOSView != NULL ? (*opq->api.itf.allocFramebuffFromOSView)(opq->api.itfParam, mtkView) : (ScnGpuFramebuffRef)ScnObjRef_Zero);
}

ScnBOOL ScnGpuDevice_render(ScnGpuDeviceRef ref, ScnGpuBufferRef fbPropsBuff, ScnGpuBufferRef mdlsPropsBuff, const struct STScnRenderCmd* const cmds, const ScnUI32 cmdsSz){
    STScnGpuDeviceOpq* opq = (STScnGpuDeviceOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq->api.itf.render != NULL ? (*opq->api.itf.render)(opq->api.itfParam, fbPropsBuff, mdlsPropsBuff, cmds, cmdsSz) : ScnFALSE);
}

