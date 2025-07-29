//
//  ScnGpuVertexbuff.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/gpu/ScnGpuVertexbuff.h"
#include "ixrender/gpu/ScnGpuBuffer.h"

//STScnGpuVertexbuffOpq

typedef struct STScnGpuVertexbuffOpq_ {
    //api
    struct {
        STScnGpuVertexbuffApiItf itf;
        void*               itfParam;
    } api;
} STScnGpuVertexbuffOpq;

ScnSI32 ScnGpuVertexbuff_getOpqSz(void){
    return (ScnSI32)sizeof(STScnGpuVertexbuffOpq);
}

void ScnGpuVertexbuff_initZeroedOpq(STScnContextRef ctx, void* obj) {
    //STScnGpuVertexbuffOpq* opq = (STScnGpuVertexbuffOpq*)obj;
}

void ScnGpuVertexbuff_destroyOpq(void* obj){
    STScnGpuVertexbuffOpq* opq = (STScnGpuVertexbuffOpq*)obj;
    //api
    {
        if(opq->api.itf.free != NULL){
            (*opq->api.itf.free)(opq->api.itfParam);
        }
        ScnMemory_setZeroSt(opq->api.itf, STScnGpuVertexbuffApiItf);
        opq->api.itfParam = NULL;
    }
}

//

ScnBOOL ScnGpuVertexbuff_prepare(STScnGpuVertexbuffRef ref, const STScnGpuVertexbuffApiItf* itf, void* itfParam) {
    ScnBOOL r = ScnFALSE;
    STScnGpuVertexbuffOpq* opq = (STScnGpuVertexbuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
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

ScnBOOL ScnGpuVertexbuff_sync(STScnGpuVertexbuffRef ref, const STScnGpuVertexbuffCfg* cfg, STScnGpuBufferRef vBuff, STScnGpuBufferRef idxBuff){
    STScnGpuVertexbuffOpq* opq = (STScnGpuVertexbuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq != NULL && opq->api.itf.sync != NULL ? (*opq->api.itf.sync)(opq->api.itfParam, cfg, vBuff, idxBuff) : ScnFALSE);
}

ScnBOOL ScnGpuVertexbuff_activate(STScnGpuVertexbuffRef ref){
    STScnGpuVertexbuffOpq* opq = (STScnGpuVertexbuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq != NULL && opq->api.itf.activate != NULL ? (*opq->api.itf.activate)(opq->api.itfParam) : ScnFALSE);
}

ScnBOOL ScnGpuVertexbuff_deactivate(STScnGpuVertexbuffRef ref){
    STScnGpuVertexbuffOpq* opq = (STScnGpuVertexbuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq != NULL && opq->api.itf.deactivate != NULL ? (*opq->api.itf.deactivate)(opq->api.itfParam) : ScnFALSE);
}
