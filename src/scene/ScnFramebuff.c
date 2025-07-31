//
//  ScnFramebuff.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/scene/ScnFramebuff.h"
#include "ixrender/gpu/ScnGpuFramebuff.h"

//STScnFramebuffOpq

typedef struct STScnFramebuffOpq {
    ScnContextRef     ctx;
    ScnMutexRef       mutex;
    //
    ScnGpuDeviceRef     gpuDev;
    ScnGpuFramebuffRef  gpuFramebuff;
    //binding
    struct {
        ENScnGpuFramebuffDstType type;
    } binding;
} STScnFramebuffOpq;

ScnSI32 ScnFramebuff_getOpqSz(void){
    return (ScnSI32)sizeof(STScnFramebuffOpq);
}

void ScnFramebuff_initZeroedOpq(ScnContextRef ctx, void* obj) {
    STScnFramebuffOpq* opq = (STScnFramebuffOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex = ScnContext_allocMutex(opq->ctx);
}

void ScnFramebuff_destroyOpq(void* obj){
    STScnFramebuffOpq* opq = (STScnFramebuffOpq*)obj;
    {
        switch(opq->binding.type){
            case ENScnGpuFramebuffDstType_None:
                break;
            case ENScnGpuFramebuffDstType_OSView:
                //nothing
                break;
            default:
                SCN_ASSERT(ScnFALSE) //unexpected value
                break;
        }
    }
    ScnGpuFramebuff_releaseAndNull(&opq->gpuFramebuff);
    ScnGpuDevice_releaseAndNull(&opq->gpuDev);
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNull(&opq->ctx);
}

//

ScnBOOL ScnFramebuff_prepare(ScnFramebuffRef ref, ScnGpuDeviceRef gpuDev) {
    ScnBOOL r = ScnFALSE;
    STScnFramebuffOpq* opq = (STScnFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(ScnGpuDevice_isNull(opq->gpuDev)){
        ScnGpuDevice_set(&opq->gpuDev, gpuDev);
        r = ScnTRUE;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//binding

ScnBOOL ScnFramebuff_bindToOSView(ScnFramebuffRef ref, void* mtkView){
    ScnBOOL r = ScnFALSE;
    STScnFramebuffOpq* opq = (STScnFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnGpuDevice_isNull(opq->gpuDev) && opq->binding.type == ENScnGpuFramebuffDstType_None){
        ScnGpuFramebuffRef gpuFb = ScnGpuDevice_allocFramebuffFromOSView(opq->gpuDev, mtkView);
        if(ScnGpuFramebuff_isNull(gpuFb)){
            printf("ERROR, ScnFramebuff_bindToOSView::ScnGpuDevice_allocFramebuffFromOSView failed.\n");
        } else {
            opq->binding.type = ENScnGpuFramebuffDstType_OSView;
            ScnGpuFramebuff_set(&opq->gpuFramebuff, gpuFb);
            r = ScnTRUE;
        }
        ScnGpuFramebuff_releaseAndNull(&gpuFb);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//

STScnSize2DU  ScnFramebuff_getSize(ScnFramebuffRef ref, STScnRectU* dstViewport){
    STScnSize2DU r = (STScnSize2DU)STScnSize2DU_Zero;
    STScnRectU rr = (STScnRectU)STScnRectU_Zero;
    STScnFramebuffOpq* opq = (STScnFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnGpuFramebuff_isNull(opq->gpuFramebuff)){
        r = ScnGpuFramebuff_getSize(opq->gpuFramebuff, &rr);
    }
    ScnMutex_unlock(opq->mutex);
    if(dstViewport != NULL) *dstViewport = rr;
    return r;
}

ScnBOOL ScnFramebuff_syncSizeAndViewport(ScnFramebuffRef ref, const STScnSize2DU size, const STScnRectU viewport){
    ScnBOOL r = ScnFALSE;
    STScnFramebuffOpq* opq = (STScnFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnGpuFramebuff_isNull(opq->gpuFramebuff)){
        r = ScnGpuFramebuff_syncSizeAndViewport(opq->gpuFramebuff, size, viewport);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}


//gpu

ScnBOOL ScnFramebuff_prepareCurrentRenderSlot(ScnFramebuffRef ref){
    ScnBOOL r = ScnFALSE;
    STScnFramebuffOpq* opq = (STScnFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnGpuFramebuff_isNull(opq->gpuFramebuff)){
        if(opq->binding.type == ENScnGpuFramebuffDstType_OSView){
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnFramebuff_moveToNextRenderSlot(ScnFramebuffRef ref){
    ScnBOOL r = ScnFALSE;
    STScnFramebuffOpq* opq = (STScnFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnGpuFramebuff_isNull(opq->gpuFramebuff)){
        if(opq->binding.type == ENScnGpuFramebuffDstType_OSView){
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}
