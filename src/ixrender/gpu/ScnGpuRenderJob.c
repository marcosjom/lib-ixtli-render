//
//  ScnGpuRenderJob.c
//  ixtli-render
//
//  Created by Marcos Ortega on 10/8/25.
//

#include "ixrender/gpu/ScnGpuRenderJob.h"
#include "ixrender/core/ScnArraySorted.h"

//STScnGpuRenderJobOpq

typedef struct STScnGpuRenderJobOpq {
    //api
    struct {
        STScnGpuRenderJobApiItf itf;
        void*           itfParam;
    } api;
} STScnGpuRenderJobOpq;

ScnUI32 ScnGpuRenderJob_getOpqSz(void){
    return (ScnUI32)sizeof(STScnGpuRenderJobOpq);
}

void ScnGpuRenderJob_initZeroedOpq(ScnContextRef ctx, void* obj) {
    //STScnGpuRenderJobOpq* opq = (STScnGpuRenderJobOpq*)obj;
}

void ScnGpuRenderJob_destroyOpq(void* obj){
    STScnGpuRenderJobOpq* opq = (STScnGpuRenderJobOpq*)obj;
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

ScnBOOL ScnGpuRenderJob_prepare(ScnGpuRenderJobRef ref, const STScnGpuRenderJobApiItf* itf, void* itfParam) {
    ScnBOOL r = ScnFALSE;
    STScnGpuRenderJobOpq* opq = (STScnGpuRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
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

void* ScnGpuRenderJob_getApiItfParam(ScnGpuRenderJobRef ref){
    STScnGpuRenderJobOpq* opq = (STScnGpuRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->api.itfParam;
}

ENScnGpuRenderJobState ScnGpuRenderJob_getState(ScnGpuRenderJobRef ref){
    STScnGpuRenderJobOpq* opq = (STScnGpuRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq != NULL && opq->api.itf.getState != NULL ? (*opq->api.itf.getState)(opq->api.itfParam) : ENScnGpuRenderJobState_Unknown);
}

ScnBOOL ScnGpuRenderJob_buildBegin(ScnGpuRenderJobRef ref, ScnGpuBufferRef bPropsScns, ScnGpuBufferRef bPropsMdls){
    STScnGpuRenderJobOpq* opq = (STScnGpuRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq != NULL && opq->api.itf.buildBegin != NULL ? (*opq->api.itf.buildBegin)(opq->api.itfParam, bPropsScns, bPropsMdls) : ScnFALSE);
}

ScnBOOL ScnGpuRenderJob_buildAddCmds(ScnGpuRenderJobRef ref, const struct STScnRenderCmd* const cmds, const ScnUI32 cmdsSz){
    STScnGpuRenderJobOpq* opq = (STScnGpuRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq != NULL && opq->api.itf.buildAddCmds != NULL ? (*opq->api.itf.buildAddCmds)(opq->api.itfParam, cmds, cmdsSz) : ScnFALSE);
}

ScnBOOL ScnGpuRenderJob_buildEndAndEnqueue(ScnGpuRenderJobRef ref){
    STScnGpuRenderJobOpq* opq = (STScnGpuRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq != NULL && opq->api.itf.buildEndAndEnqueue != NULL ? (*opq->api.itf.buildEndAndEnqueue)(opq->api.itfParam) : ScnFALSE);
}
