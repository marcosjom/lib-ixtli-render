//
//  ScnTexture.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/scene/ScnTexture.h"
#include "ixrender/core/ScnArray.h"
#include "ixrender/type/ScnColor.h"

//STScnTextureOpq

typedef struct STScnTextureOpq_ {
    STScnContextRef     ctx;
    STScnMutexRef       mutex;
    //
    STScnGpuTextureCfg     cfg;    //config
    STScnBitmapRef      bmp;    //bitmap
    //changes
    struct {
        ScnBOOL         whole;  //all the content is new
        ScnArrayStruct(rects, STScnRectI); //subimages areas
    } changes;
} STScnTextureOpq;

ScnSI32 ScnTexture_getOpqSz(void){
    return (ScnSI32)sizeof(STScnTextureOpq);
}

void ScnTexture_initZeroedOpq(STScnContextRef ctx, void* obj) {
    STScnTextureOpq* opq = (STScnTextureOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex = ScnContext_allocMutex(opq->ctx);
    //changes
    {
        ScnArray_init(opq->ctx, &opq->changes.rects, 0, 32, STScnRectI);
    }
}

void ScnTexture_destroyOpq(void* obj){
    STScnTextureOpq* opq = (STScnTextureOpq*)obj;
    //
    //ScnStruct_stRelease(ScnTextureCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
    //
    if(!ScnBitmap_isNull(opq->bmp)){
        ScnBitmap_release(&opq->bmp);
        ScnBitmap_null(&opq->bmp);
    }
    //changes
    {
        ScnArray_destroy(opq->ctx, &opq->changes.rects);
    }
    //
    if(!ScnMutex_isNull(opq->mutex)){
        ScnMutex_free(&opq->mutex);
        ScnMutex_null(&opq->mutex);
    }
    //
    if(!ScnContext_isNull(opq->ctx)){
        ScnContext_release(&opq->ctx);
        ScnContext_null(&opq->ctx);
    }
}

//

ScnBOOL ScnTexture_prepare(STScnTextureRef ref, const STScnGpuTextureCfg* cfg) {
    ScnBOOL r = ScnFALSE;
    STScnTextureOpq* opq = (STScnTextureOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(cfg != NULL && cfg->width > 0 && cfg->height > 0 && opq->cfg.width == 0){
        STScnBitmapRef bmp = ScnBitmap_alloc(opq->ctx);
        if(!ScnBitmap_create(bmp, cfg->width, cfg->height, cfg->color)){
            //error
        } else {
            //swap
            ScnBitmap_set(&opq->bmp, bmp);
            //cfg
            opq->cfg = *cfg;
            //ScnStruct_stRelease(ScnTextureCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
            //ScnStruct_stClone(ScnTextureCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
            //changes
            opq->changes.whole = ScnTRUE;
            ScnArray_empty(&opq->changes.rects);
            //
            r = ScnTRUE;
        }
        ScnBitmap_release(&bmp);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}


ScnBOOL ScnTexture_setImage(STScnTextureRef ref, const STScnBitmapProps srcProps, const ScnBYTE* srcData){
    ScnBOOL r = ScnFALSE;
    STScnTextureOpq* opq = (STScnTextureOpq*)ScnSharedPtr_getOpq(ref.ptr);
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

ScnBOOL ScnTexture_setSubimage(STScnTextureRef ref, const STScnPointI pos, const STScnBitmapProps srcProps, const ScnBYTE* srcData, const STScnRectI pSrcRect){
    ScnBOOL r = ScnFALSE;
    STScnTextureOpq* opq = (STScnTextureOpq*)ScnSharedPtr_getOpq(ref.ptr);
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
