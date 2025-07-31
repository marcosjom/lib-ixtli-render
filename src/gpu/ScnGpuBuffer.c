//
//  ScnGpuBuffer.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/gpu/ScnGpuBuffer.h"
#include "ixrender/core/ScnArraySorted.h"

//STScnGpuBufferOpq

typedef struct STScnGpuBufferOpq {
    //api
    struct {
        STScnGpuBufferApiItf itf;
        void*           itfParam;
    } api;
} STScnGpuBufferOpq;

ScnSI32 ScnGpuBuffer_getOpqSz(void){
    return (ScnSI32)sizeof(STScnGpuBufferOpq);
}

void ScnGpuBuffer_initZeroedOpq(ScnContextRef ctx, void* obj) {
    //STScnGpuBufferOpq* opq = (STScnGpuBufferOpq*)obj;
}

void ScnGpuBuffer_destroyOpq(void* obj){
    STScnGpuBufferOpq* opq = (STScnGpuBufferOpq*)obj;
    //api
    {
        if(opq->api.itf.free != NULL){
            (*opq->api.itf.free)(opq->api.itfParam);
        }
        ScnMemory_setZeroSt(opq->api.itf, STScnGpuBufferApiItf);
        opq->api.itfParam = NULL;
    }
}

//

ScnBOOL ScnGpuBuffer_prepare(ScnGpuBufferRef ref, const STScnGpuBufferApiItf* itf, void* itfParam) {
    ScnBOOL r = ScnFALSE;
    STScnGpuBufferOpq* opq = (STScnGpuBufferOpq*)ScnSharedPtr_getOpq(ref.ptr);
    if(itf != NULL && itf->free != NULL){
        //api
        {
            if(opq->api.itf.free != NULL){
                (*opq->api.itf.free)(opq->api.itfParam);
            }
            ScnMemory_setZeroSt(opq->api.itf, STScnGpuBufferApiItf);
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

ScnBOOL ScnGpuBuffer_sync(ScnGpuBufferRef ref, const STScnGpuBufferCfg* cfg, ScnMemElasticRef mem, const STScnGpuBufferChanges* changes){
    STScnGpuBufferOpq* opq = (STScnGpuBufferOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq != NULL && opq->api.itf.sync != NULL ? (*opq->api.itf.sync)(opq->api.itfParam, cfg, mem, changes) : ScnFALSE);
}
