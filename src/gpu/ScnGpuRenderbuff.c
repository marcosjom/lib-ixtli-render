//
//  ScnGpuRenderbuff.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/gpu/ScnGpuRenderbuff.h"

//STScnGpuRenderbuffOpq

typedef struct STScnGpuRenderbuffOpq {
    ScnContextRef     ctx;
    ScnMutexRef       mutex;
    //
    STScnGpuRenderbuffCfg cfg;    //config
    //api
    struct {
        STScnGpuRenderbuffApiItf itf;
        void*           itfParam;
        void*           data;
    } api;
} STScnGpuRenderbuffOpq;

//

ScnSI32 ScnGpuRenderbuff_getOpqSz(void){
    return (ScnSI32)sizeof(STScnGpuRenderbuffOpq);
}

void ScnGpuRenderbuff_initZeroedOpq(ScnContextRef ctx, void* obj) {
    STScnGpuRenderbuffOpq* opq = (STScnGpuRenderbuffOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex = ScnContext_allocMutex(opq->ctx);

}

void ScnGpuRenderbuff_destroyOpq(void* obj){
    STScnGpuRenderbuffOpq* opq = (STScnGpuRenderbuffOpq*)obj;
    //api
    {
        if(opq->api.data != NULL){
            if(opq->api.itf.destroy != NULL){
                (*opq->api.itf.destroy)(opq->api.data, opq->api.itfParam);
            }
            opq->api.data = NULL;
        }
        ScnMemory_setZeroSt(opq->api.itf, STScnGpuRenderbuffApiItf);
        opq->api.itfParam = NULL;
    }
    //
    //ScnStruct_stRelease(ScnGpuRenderbuffCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
    //
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNull(&opq->ctx);
}


//

ScnBOOL ScnGpuRenderbuff_prepare(ScnGpuRenderbuffRef ref, const STScnGpuRenderbuffCfg* cfg, const STScnGpuRenderbuffApiItf* itf, void* itfParam) {
    ScnBOOL r = ScnFALSE;
    STScnGpuRenderbuffOpq* opq = (STScnGpuRenderbuffOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(cfg != NULL && cfg->width > 0 && cfg->height > 0 && opq->cfg.width == 0 && itf != NULL && itf->create != NULL && itf->destroy != NULL){
        void* data = (*itf->create)(cfg, itfParam);
        if(data != NULL){
            opq->cfg = *cfg;
            //ScnStruct_stRelease(ScnGpuRenderbuffCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
            //ScnStruct_stClone(ScnGpuRenderbuffCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
            //api
            {
                if(opq->api.data != NULL){
                    if(opq->api.itf.destroy != NULL){
                        (*opq->api.itf.destroy)(opq->api.data, opq->api.itfParam);
                    }
                    opq->api.data = NULL;
                }
                ScnMemory_setZeroSt(opq->api.itf, STScnGpuRenderbuffApiItf);
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
