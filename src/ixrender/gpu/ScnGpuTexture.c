//
//  ScnGpuTexture.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/gpu/ScnGpuTexture.h"
#include "ixrender/core/ScnArray.h"
#include "ixrender/type/ScnColor.h"

//STScnGpuTextureOpq

typedef struct STScnGpuTextureOpq {
    //api
    struct {
        STScnGpuTextureApiItf itf;
        void*           itfParam;
    } api;
} STScnGpuTextureOpq;

ScnUI32 ScnGpuTexture_getOpqSz(void){
    return (ScnUI32)sizeof(STScnGpuTextureOpq);
}

void ScnGpuTexture_initZeroedOpq(ScnContextRef ctx, void* obj) {
    //STScnGpuTextureOpq* opq = (STScnGpuTextureOpq*)obj;
}

void ScnGpuTexture_destroyOpq(void* obj){
    STScnGpuTextureOpq* opq = (STScnGpuTextureOpq*)obj;
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

ScnBOOL ScnGpuTexture_prepare(ScnGpuTextureRef ref, const STScnGpuTextureApiItf* itf, void* itfParam){
    ScnBOOL r = ScnFALSE;
    STScnGpuTextureOpq* opq = (STScnGpuTextureOpq*)ScnSharedPtr_getOpq(ref.ptr);
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

void* ScnGpuTexture_getApiItfParam(ScnGpuTextureRef ref){
    STScnGpuTextureOpq* opq = (STScnGpuTextureOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->api.itfParam;
}


ScnBOOL ScnGpuTexture_sync(ScnGpuTextureRef ref, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData, const STScnGpuTextureChanges* const changes){
    STScnGpuTextureOpq* opq = (STScnGpuTextureOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (opq->api.itf.sync != NULL ? (*opq->api.itf.sync)(opq->api.itfParam, cfg, srcProps, srcData, changes) : ScnFALSE);
}
