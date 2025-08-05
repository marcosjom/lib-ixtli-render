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

STScnSize2DU ScnFramebuff_getSize(ScnFramebuffRef ref){
    STScnSize2DU r = (STScnSize2DU)STScnSize2DU_Zero;
    STScnFramebuffOpq* opq = (STScnFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnGpuFramebuff_isNull(opq->gpuFramebuff)){
        r = ScnGpuFramebuff_getSize(opq->gpuFramebuff);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnFramebuff_syncSize(ScnFramebuffRef ref, const STScnSize2DU size){
    ScnBOOL r = ScnFALSE;
    STScnFramebuffOpq* opq = (STScnFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnGpuFramebuff_isNull(opq->gpuFramebuff)){
        r = ScnGpuFramebuff_syncSize(opq->gpuFramebuff, size);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

STScnGpuFramebuffProps ScnFramebuff_getProps(ScnFramebuffRef ref){
    STScnGpuFramebuffProps r = (STScnGpuFramebuffProps)STScnGpuFramebuffProps_Zero;
    STScnFramebuffOpq* opq = (STScnFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnGpuFramebuff_isNull(opq->gpuFramebuff)){
        r = ScnGpuFramebuff_getProps(opq->gpuFramebuff);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnFramebuff_setProps(ScnFramebuffRef ref, const STScnGpuFramebuffProps* const props){
    ScnBOOL r = ScnFALSE;
    STScnFramebuffOpq* opq = (STScnFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnGpuFramebuff_isNull(opq->gpuFramebuff)){
        r = ScnGpuFramebuff_setProps(opq->gpuFramebuff, props);
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

ScnGpuFramebuffRef ScnFramebuff_getCurrentRenderSlot(ScnFramebuffRef ref){
    STScnFramebuffOpq* opq = (STScnFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->gpuFramebuff;
}
