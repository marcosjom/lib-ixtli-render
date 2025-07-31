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

typedef struct STScnGpuTextureOpq_ {
    ScnContextRef     ctx;
    ScnMutexRef       mutex;
    //
    STScnGpuTextureCfg  cfg;    //config
    ScnBitmapRef      bmp;    //bitmap
    //changes
    struct {
        ScnBOOL         whole;  //all the content is new
        ScnArrayStruct(rects, STScnRectI); //subimages areas
    } changes;
    //api
    struct {
        STScnGpuTextureApiItf itf;
        void*           itfParam;
        void*           data;
    } api;
} STScnGpuTextureOpq;

ScnSI32 ScnGpuTexture_getOpqSz(void){
    return (ScnSI32)sizeof(STScnGpuTextureOpq);
}

void ScnGpuTexture_initZeroedOpq(ScnContextRef ctx, void* obj) {
    STScnGpuTextureOpq* opq = (STScnGpuTextureOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex = ScnContext_allocMutex(opq->ctx);
    //changes
    {
        ScnArray_init(opq->ctx, &opq->changes.rects, 0, 32, STScnRectI);
    }
}

void ScnGpuTexture_destroyOpq(void* obj){
    STScnGpuTextureOpq* opq = (STScnGpuTextureOpq*)obj;
    //api
    {
        if(opq->api.data != NULL){
            if(opq->api.itf.destroy != NULL){
                (*opq->api.itf.destroy)(opq->api.data, opq->api.itfParam);
            }
            opq->api.data = NULL;
        }
        ScnMemory_setZeroSt(opq->api.itf, STScnGpuTextureApiItf);
        opq->api.itfParam = NULL;
    }
    //
    //ScnStruct_stRelease(ScnGpuTextureCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
    //
    ScnBitmap_releaseAndNull(&opq->bmp);
    //changes
    {
        ScnArray_destroy(opq->ctx, &opq->changes.rects);
    }
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNull(&opq->ctx);
}

//

ScnBOOL ScnGpuTexture_prepare(ScnGpuTextureRef ref, const STScnGpuTextureCfg* cfg, const STScnGpuTextureApiItf* itf, void* itfParam) {
    ScnBOOL r = ScnFALSE;
    STScnGpuTextureOpq* opq = (STScnGpuTextureOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(cfg != NULL && cfg->width > 0 && cfg->height > 0 && opq->cfg.width == 0 && itf != NULL && itf->create != NULL && itf->destroy != NULL){
        void* data = (*itf->create)(cfg, itfParam);
        if(data != NULL){
            ScnBitmapRef bmp = ScnBitmap_alloc(opq->ctx);
            if(!ScnBitmap_create(bmp, cfg->width, cfg->height, cfg->color)){
                //error
            } else {
                //swap
                ScnBitmap_set(&opq->bmp, bmp);
                //cfg
                opq->cfg = *cfg;
                //ScnStruct_stRelease(ScnGpuTextureCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
                //ScnStruct_stClone(ScnGpuTextureCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
                //changes
                opq->changes.whole = ScnTRUE;
                ScnArray_empty(&opq->changes.rects);
                //api
                {
                    if(opq->api.data != NULL){
                        if(opq->api.itf.destroy != NULL){
                            (*opq->api.itf.destroy)(opq->api.data, opq->api.itfParam);
                        }
                        opq->api.data = NULL;
                    }
                    ScnMemory_setZeroSt(opq->api.itf, STScnGpuTextureApiItf);
                    opq->api.itfParam = NULL;
                    //
                    if(itf != NULL){
                        opq->api.itf = *itf;
                        opq->api.itfParam = itfParam;
                    }
                    //data
                    opq->api.data = data; data = NULL; //consume
                }
                //
                r = ScnTRUE;
            }
            ScnBitmap_release(&bmp);
        }
        //destroy (if not consumed)
        if(data != NULL && itf->destroy != NULL){
            (*itf->destroy)(data, itfParam);
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}


ScnBOOL ScnGpuTexture_setImage(ScnGpuTextureRef ref, const STScnBitmapProps srcProps, const ScnBYTE* srcData){
    ScnBOOL r = ScnFALSE;
    STScnGpuTextureOpq* opq = (STScnGpuTextureOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->cfg.width == srcProps.size.width && opq->cfg.height == srcProps.size.height){
        const STScnPointI pstPos = { 0, 0 };
        if(!ScnBitmap_pasteBitmapData(opq->bmp, pstPos, srcProps, srcData)){
            //error
        } else {
            //changes
            opq->changes.whole = ScnTRUE;
            ScnArray_empty(&opq->changes.rects);
            //
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnGpuTexture_setSubimage(ScnGpuTextureRef ref, const STScnPointI pos, const STScnBitmapProps srcProps, const ScnBYTE* srcData, const STScnRectI pSrcRect){
    ScnBOOL r = ScnFALSE;
    STScnGpuTextureOpq* opq = (STScnGpuTextureOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->cfg.width >0 && opq->cfg.height > 0){
        /*STScnRectI srcRect = pSrcRect;
        const STScnColor8 pstColor = { 255, 255, 255, 255 };
        if(!ScnBitmap_pasteValidatedSrcRect(&opq->bmp, pos, srcProps, &srcRect)){
            //error
        } else if(!ScnBitmap_pasteBitmapDataRect(&opq->bmp, pos, srcProps, srcData, srcRect, pstColor)){
            //error
        } else {
            //changes
            if(opq->changes.whole){
                //already flagged
            } else if(opq->cfg.width == srcRect.width && opq->cfg.height == srcRect.height){
                //whole image changed
                opq->changes.whole = ScnTRUE;
                ScnArray_empty(&opq->changes.rects);
            } else {
                //add rect
                ScnArray_addValue(opq->ctx, &opq->changes.rects, srcRect, STScnRectI);
            }
            //
            r = ScnTRUE;
        }*/
        SCN_ASSERT(ScnFALSE) //implement
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}
