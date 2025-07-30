//
//  ScnFramebuff.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/scene/ScnFramebuff.h"
#include "ixrender/gpu/ScnGpuFramebuff.h"

//STScnFramebuffOpq

typedef struct STScnFramebuffOpq_ {
    STScnContextRef     ctx;
    STScnMutexRef       mutex;
    //
    STScnGpuDeviceRef   gpuDev;
} STScnFramebuffOpq;

ScnSI32 ScnFramebuff_getOpqSz(void){
    return (ScnSI32)sizeof(STScnFramebuffOpq);
}

void ScnFramebuff_initZeroedOpq(STScnContextRef ctx, void* obj) {
    STScnFramebuffOpq* opq = (STScnFramebuffOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex = ScnContext_allocMutex(opq->ctx);
}

void ScnFramebuff_destroyOpq(void* obj){
    STScnFramebuffOpq* opq = (STScnFramebuffOpq*)obj;
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

ScnBOOL ScnFramebuff_prepare(STScnFramebuffRef ref, STScnGpuDeviceRef gpuDev, const ScnUI32 ammRenderSlots) {
    ScnBOOL r = ScnFALSE;
    STScnFramebuffOpq* opq = (STScnFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        //
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}
