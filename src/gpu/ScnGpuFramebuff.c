//
//  ScnGpuFramebuff.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/gpu/ScnGpuFramebuff.h"

//STScnGpuFramebuffOpq

typedef struct STScnGpuFramebuffOpq_ {
    ScnContextRef     ctx;
    ScnMutexRef       mutex;
    //
    STScnGpuFramebuffCfg cfg;    //config
    //bind
    struct {
        ENScnGpuFramebuffDstType type;
        ScnObjRef     ref;    //ScnGpuTextureRef, ScnGpuRenderbuffRef
    } bind;
    //changes
    struct {
        ScnBOOL         bind;
    } changes;
    //api
    struct {
        STScnGpuFramebuffApiItf itf;
        void*           itfParam;
        void*           data;
    } api;
} STScnGpuFramebuffOpq;

ScnSI32 ScnGpuFramebuff_getOpqSz(void){
    return (ScnSI32)sizeof(STScnGpuFramebuffOpq);
}

void ScnGpuFramebuff_initZeroedOpq(ScnContextRef ctx, void* obj) {
    STScnGpuFramebuffOpq* opq = (STScnGpuFramebuffOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex = ScnContext_allocMutex(opq->ctx);

}

void ScnGpuFramebuff_destroyOpq(void* obj){
    STScnGpuFramebuffOpq* opq = (STScnGpuFramebuffOpq*)obj;
    //api
    {
        if(opq->api.data != NULL){
            if(opq->api.itf.destroy != NULL){
                (*opq->api.itf.destroy)(opq->api.data, opq->api.itfParam);
            }
            opq->api.data = NULL;
        }
        ScnMemory_setZeroSt(opq->api.itf, STScnGpuFramebuffApiItf);
        opq->api.itfParam = NULL;
    }
    //
    //ScnStruct_stRelease(ScnGpuFramebuffCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
    //bind
    ScnObjRef_releaseAndNullify(&opq->bind.ref);
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNullify(&opq->ctx);
}

//

ScnBOOL ScnGpuFramebuff_prepare(ScnGpuFramebuffRef ref, const STScnGpuFramebuffCfg* cfg, const STScnGpuFramebuffApiItf* itf, void* itfParam) {
    ScnBOOL r = ScnFALSE;
    STScnGpuFramebuffOpq* opq = (STScnGpuFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(cfg != NULL && itf != NULL && itf->create != NULL && itf->destroy != NULL){
        void* data = (*itf->create)(cfg, itfParam);
        if(data != NULL){
            opq->cfg = *cfg;
            //ScnStruct_stRelease(ScnGpuFramebuffCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
            //ScnStruct_stClone(ScnGpuFramebuffCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
            //api
            {
                if(opq->api.data != NULL){
                    if(opq->api.itf.destroy != NULL){
                        (*opq->api.itf.destroy)(opq->api.data, opq->api.itfParam);
                    }
                    opq->api.data = NULL;
                }
                ScnMemory_setZeroSt(opq->api.itf, STScnGpuFramebuffApiItf);
                opq->api.itfParam = NULL;
                //
                if(itf != NULL){
                    opq->api.itf = *itf;
                    opq->api.itfParam = itfParam;
                }
                //data
                opq->api.data = data; data = NULL; //consume
            }
        }
        //destroy (if not consumed)
        if(data != NULL && itf->destroy != NULL){
            (*itf->destroy)(data, itfParam);
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnGpuFramebuff_bindTo(ScnGpuFramebuffRef ref, const ScnObjRef dstRef, const ENScnGpuFramebuffDstType type){
    ScnBOOL r = ScnFALSE;
    STScnGpuFramebuffOpq* opq = (STScnGpuFramebuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        if(ScnObjRef_isNull(dstRef)){
            opq->bind.type = ENScnGpuFramebuffDstType_None;
            if(!ScnObjRef_isNull(opq->bind.ref)){
                ScnObjRef_release(&opq->bind.ref);
                ScnObjRef_null(&opq->bind.ref);
                opq->changes.bind = ScnTRUE;
            }
            r = ScnTRUE;
        } else if(type == ENScnGpuFramebuffDstType_Texture){
            opq->bind.type = ENScnGpuFramebuffDstType_Texture;
            if(!ScnObjRef_isSame(opq->bind.ref, dstRef)){
                ScnObjRef_set(&opq->bind.ref, dstRef);
                opq->changes.bind = ScnTRUE;
            }
            r = ScnTRUE;
        } else if(type == ENScnGpuFramebuffDstType_Renderbuffer){
            opq->bind.type = ENScnGpuFramebuffDstType_Renderbuffer;
            if(!ScnObjRef_isSame(opq->bind.ref, dstRef)){
                ScnObjRef_set(&opq->bind.ref, dstRef);
                opq->changes.bind = ScnTRUE;
            }
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}
