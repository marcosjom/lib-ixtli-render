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
    ScnGpuDevice_releaseAndNullify(&opq->gpuDev);
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNullify(&opq->ctx);
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
