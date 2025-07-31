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
    ScnContextRef       ctx;
    ScnMutexRef         mutex;
    //
    STScnGpuTextureCfg  cfg;    //config
    ScnBitmapRef        bmp;    //bitmap
    //changes
    struct {
        ScnBOOL         whole;  //all the content is new
        ScnArrayStruct(rects, STScnRectI); //subimages areas
    } changes;
} STScnTextureOpq;

ScnSI32 ScnTexture_getOpqSz(void){
    return (ScnSI32)sizeof(STScnTextureOpq);
}

void ScnTexture_initZeroedOpq(ScnContextRef ctx, void* obj) {
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
    ScnBitmap_releaseAndNull(&opq->bmp);
    //changes
    {
        ScnArray_destroy(opq->ctx, &opq->changes.rects);
    }
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNull(&opq->ctx);
}

//

ScnBOOL ScnTexture_prepare(ScnTextureRef ref, const STScnGpuTextureCfg* cfg) {
    ScnBOOL r = ScnFALSE;
    STScnTextureOpq* opq = (STScnTextureOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(cfg != NULL && cfg->width > 0 && cfg->height > 0 && opq->cfg.width == 0){
        ScnBitmapRef bmp = ScnBitmap_alloc(opq->ctx);
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


ScnBOOL ScnTexture_setImage(ScnTextureRef ref, const STScnBitmapProps srcProps, const ScnBYTE* srcData){
    ScnBOOL r = ScnFALSE;
    STScnTextureOpq* opq = (STScnTextureOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->cfg.width == srcProps.size.width && opq->cfg.height == srcProps.size.height){
        const STScnPoint2DI pstPos = { 0, 0 };
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

ScnBOOL ScnTexture_setSubimage(ScnTextureRef ref, const STScnPoint2DI pos, const STScnBitmapProps srcProps, const ScnBYTE* srcData, const STScnRectI pSrcRect){
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
