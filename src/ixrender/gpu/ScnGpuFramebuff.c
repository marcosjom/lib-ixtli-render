//
//  ScnGpuFramebuff.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/gpu/ScnGpuFramebuff.h"

//STScnGpuFramebuffOpq

typedef struct STScnGpuFramebuffOpq {
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
        ScnMemory_setZeroSt(opq->api.itf);
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

void* ScnGpuFramebuff_getApiItfParam(ScnGpuFramebuffRef ref){
    STScnGpuFramebuffOpq* opq = (STScnGpuFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->api.itfParam;
}

STScnSize2DU ScnGpuFramebuff_getSize(ScnGpuFramebuffRef ref){
    STScnGpuFramebuffOpq* opq = (STScnGpuFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq != NULL && opq->api.itf.getSize != NULL ? (*opq->api.itf.getSize)(opq->api.itfParam) : (STScnSize2DU)STScnSize2DU_Zero );
}

ScnBOOL ScnGpuFramebuff_syncSize(ScnGpuFramebuffRef ref, const STScnSize2DU size){
    STScnGpuFramebuffOpq* opq = (STScnGpuFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq != NULL && opq->api.itf.syncSize != NULL ? (*opq->api.itf.syncSize)(opq->api.itfParam, size) : ScnFALSE );
}

STScnGpuFramebuffProps ScnGpuFramebuff_getProps(ScnGpuFramebuffRef ref){
    STScnGpuFramebuffOpq* opq = (STScnGpuFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq != NULL && opq->api.itf.getProps != NULL ? (*opq->api.itf.getProps)(opq->api.itfParam) : (STScnGpuFramebuffProps)STScnGpuFramebuffProps_Zero );
}

ScnBOOL ScnGpuFramebuff_setProps(ScnGpuFramebuffRef ref, const STScnGpuFramebuffProps* const props){
    STScnGpuFramebuffOpq* opq = (STScnGpuFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq != NULL && opq->api.itf.setProps != NULL ? (*opq->api.itf.setProps)(opq->api.itfParam, props) : ScnFALSE );
}
