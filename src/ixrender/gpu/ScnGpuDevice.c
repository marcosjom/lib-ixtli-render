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

ScnUI32 ScnGpuDevice_getOpqSz(void){
    return (ScnUI32)sizeof(STScnGpuDeviceOpq);
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
        ScnMemory_setZeroSt(opq->api.itf);
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
            ScnMemory_setZeroSt(opq->api.itf);
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

STScnGpuDeviceDesc ScnGpuDevice_getDesc(ScnGpuDeviceRef ref){
    STScnGpuDeviceOpq* opq = (STScnGpuDeviceOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq->api.itf.getDesc != NULL ? (*opq->api.itf.getDesc)(opq->api.itfParam) : (STScnGpuDeviceDesc)STScnGpuDeviceDesc_Zero);
}

ScnGpuBufferRef ScnGpuDevice_allocBuffer(ScnGpuDeviceRef ref, ScnMemElasticRef mem){
    STScnGpuDeviceOpq* opq = (STScnGpuDeviceOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq->api.itf.allocBuffer != NULL ? (*opq->api.itf.allocBuffer)(opq->api.itfParam, mem) : (ScnGpuBufferRef)ScnObjRef_Zero);
}

ScnGpuVertexbuffRef ScnGpuDevice_allocVertexBuff(ScnGpuDeviceRef ref, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff){
    STScnGpuDeviceOpq* opq = (STScnGpuDeviceOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq->api.itf.allocVertexBuff != NULL ? (*opq->api.itf.allocVertexBuff)(opq->api.itfParam, cfg, vBuff, idxBuff) : (ScnGpuVertexbuffRef)ScnObjRef_Zero);
}

ScnGpuFramebuffRef ScnGpuDevice_allocFramebuffFromOSView(ScnGpuDeviceRef ref, void* mtkView){
    STScnGpuDeviceOpq* opq = (STScnGpuDeviceOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq->api.itf.allocFramebuffFromOSView != NULL ? (*opq->api.itf.allocFramebuffFromOSView)(opq->api.itfParam, mtkView) : (ScnGpuFramebuffRef)ScnObjRef_Zero);
}

ScnGpuTextureRef ScnGpuDevice_allocTexture(ScnGpuDeviceRef ref, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData){
    STScnGpuDeviceOpq* opq = (STScnGpuDeviceOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq->api.itf.allocTexture != NULL ? (*opq->api.itf.allocTexture)(opq->api.itfParam, cfg, srcProps, srcData) : (ScnGpuTextureRef)ScnObjRef_Zero);
}

ScnGpuSamplerRef ScnGpuDevice_allocSampler(ScnGpuDeviceRef ref, const STScnGpuSamplerCfg* const cfg){
    STScnGpuDeviceOpq* opq = (STScnGpuDeviceOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq->api.itf.allocSampler != NULL ? (*opq->api.itf.allocSampler)(opq->api.itfParam, cfg) : (ScnGpuSamplerRef)ScnGpuSamplerRef_Zero);
}

ScnGpuRenderJobRef ScnGpuDevice_allocRenderJob(ScnGpuDeviceRef ref){
    STScnGpuDeviceOpq* opq = (STScnGpuDeviceOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq->api.itf.allocRenderJob != NULL ? (*opq->api.itf.allocRenderJob)(opq->api.itfParam) : (ScnGpuRenderJobRef)ScnGpuRenderJobRef_Zero);
}

