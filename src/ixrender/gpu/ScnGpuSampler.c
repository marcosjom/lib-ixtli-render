//
//  ScnSampler.c
//  nbframework
//
//  Created by Marcos Ortega on 8/8/25.
//

#include "ixrender/gpu/ScnGpuSampler.h"

//STScnGpuSamplerOpq

typedef struct STScnGpuSamplerOpq {
    //api
    struct {
        STScnGpuSamplerApiItf itf;
        void*           itfParam;
    } api;
} STScnGpuSamplerOpq;

ScnUI32 ScnGpuSampler_getOpqSz(void){
    return (ScnUI32)sizeof(STScnGpuSamplerOpq);
}

void ScnGpuSampler_initZeroedOpq(ScnContextRef ctx, void* obj) {
    //STScnGpuSamplerOpq* opq = (STScnGpuSamplerOpq*)obj;
}

void ScnGpuSampler_destroyOpq(void* obj){
    STScnGpuSamplerOpq* opq = (STScnGpuSamplerOpq*)obj;
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

ScnBOOL ScnGpuSampler_prepare(ScnGpuSamplerRef ref, const STScnGpuSamplerApiItf* itf, void* itfParam){
    ScnBOOL r = ScnFALSE;
    STScnGpuSamplerOpq* opq = (STScnGpuSamplerOpq*)ScnSharedPtr_getOpq(ref.ptr);
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
        }
        r = ScnTRUE;
    }
    return r;
}

void* ScnGpuSampler_getApiItfParam(ScnGpuSamplerRef ref){
    STScnGpuSamplerOpq* opq = (STScnGpuSamplerOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->api.itfParam;
}


STScnGpuSamplerCfg ScnGpuSampler_getCfg(ScnGpuSamplerRef ref){
    STScnGpuSamplerOpq* opq = (STScnGpuSamplerOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq->api.itf.getCfg != NULL ? (*opq->api.itf.getCfg)(opq->api.itfParam) : (STScnGpuSamplerCfg)STScnGpuSamplerCfg_Zero);
}
