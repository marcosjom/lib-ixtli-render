//
//  ScnGpuFramebuff.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/gpu/ScnGpuFramebuff.h"

//STScnGpuFramebuffOpq

typedef struct STScnGpuFramebuffOpq_ {
    //api
    struct {
        STScnGpuFramebuffApiItf itf;
        void*           itfParam;
    } api;
} STScnGpuFramebuffOpq;

ScnSI32 ScnGpuFramebuff_getOpqSz(void){
    return (ScnSI32)sizeof(STScnGpuFramebuffOpq);
}

void ScnGpuFramebuff_initZeroedOpq(ScnContextRef ctx, void* obj) {
    //STScnGpuFramebuffOpq* opq = (STScnGpuFramebuffOpq*)obj;
}

void ScnGpuFramebuff_destroyOpq(void* obj){
    STScnGpuFramebuffOpq* opq = (STScnGpuFramebuffOpq*)obj;
    //api
    {
        if(opq->api.itf.free != NULL){
            (*opq->api.itf.free)(opq->api.itfParam);
        }
        ScnMemory_setZeroSt(opq->api.itf, STScnGpuFramebuffApiItf);
        opq->api.itfParam = NULL;
    }
}

//

ScnBOOL ScnGpuFramebuff_prepare(ScnGpuFramebuffRef ref, const STScnGpuFramebuffApiItf* itf, void* itfParam) {
    ScnBOOL r = ScnFALSE;
    STScnGpuFramebuffOpq* opq = (STScnGpuFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    if(itf != NULL && itf->free != NULL){
        //api
        {
            if(opq->api.itf.free != NULL){
                (*opq->api.itf.free)(opq->api.itfParam);
            }
            ScnMemory_setZeroSt(opq->api.itf, STScnGpuFramebuffApiItf);
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

STScnSizeU ScnGpuFramebuff_getSize(ScnGpuFramebuffRef ref, STScnRectU* dstViewport){
    STScnGpuFramebuffOpq* opq = (STScnGpuFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    if(dstViewport != NULL) *dstViewport = (STScnRectU)STScnRectU_Zero;
    return (opq != NULL && opq->api.itf.getSize != NULL ? (*opq->api.itf.getSize)(opq->api.itfParam, dstViewport) : (STScnSizeU)STScnSizeU_Zero );
}

ScnBOOL ScnGpuFramebuff_syncSizeAndViewport(ScnGpuFramebuffRef ref, const STScnSizeU size, const STScnRectU viewport){
    STScnGpuFramebuffOpq* opq = (STScnGpuFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq != NULL && opq->api.itf.syncSizeAndViewport != NULL ? (*opq->api.itf.syncSizeAndViewport)(opq->api.itfParam, size, viewport) : ScnFALSE );
}
