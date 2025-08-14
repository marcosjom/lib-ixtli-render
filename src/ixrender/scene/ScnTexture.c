//
//  ScnTexture.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/scene/ScnTexture.h"
#include "ixrender/core/ScnArray.h"
#include "ixrender/core/ScnArraySorted.h"
#include "ixrender/type/ScnColor.h"
#include "ixrender/type/ScnChangeRngs.h"

//STScnTextureSlot

typedef struct STScnTextureSlot {
    ScnContextRef       ctx;
    ScnGpuTextureRef    gpuTex;
    //state
    struct {
        ScnUI32         totalSzLast;
    } state;
    STScnChangesRngs    changes;
} STScnTextureSlot;

void ScnTextureSlot_init(ScnContextRef ctx, STScnTextureSlot* opq);
void ScnTextureSlot_destroy(STScnTextureSlot* opq);

//STScnTextureOpq

typedef struct STScnTextureOpq {
    ScnContextRef       ctx;
    ScnMutexRef         mutex;
    //
    ENScnResourceMode   resMode;
    STScnGpuTextureCfg  cfg;    //config
    ScnGpuDeviceRef     gpuDev;
    STScnBitmapProps    bmpProps;
    ScnBitmapRef        bmp;    //bitmap
    STScnChangesRngs    changes;
    //slots (render)
    struct {
        STScnTextureSlot* arr;
        ScnUI16         use;
        ScnUI16         sz;
        ScnUI16         iCur;
    } slots;
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
    ScnChangesRngs_init(ctx, &opq->changes);
}

void ScnTexture_destroyOpq(void* obj){
    STScnTextureOpq* opq = (STScnTextureOpq*)obj;
    //
    //ScnStruct_stRelease(ScnTextureCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
    //
    ScnBitmap_releaseAndNull(&opq->bmp);
    //changes
    ScnChangesRngs_destroy(&opq->changes);
    //slots
    {
        if(opq->slots.arr != NULL){
            STScnTextureSlot* s = opq->slots.arr;
            const STScnTextureSlot* sAfterEnd = s + opq->slots.use;
            while(s < sAfterEnd){
                ScnTextureSlot_destroy(s);
                ++s;
            }
            ScnContext_mfree(opq->ctx, opq->slots.arr);
            opq->slots.arr = NULL;
        }
        opq->slots.use = opq->slots.sz = 0;
    }
    ScnGpuDevice_releaseAndNull(&opq->gpuDev);
    //
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNull(&opq->ctx);
}

//

ScnBOOL ScnTexture_prepare(ScnTextureRef ref, ScnGpuDeviceRef gpuDev, const ENScnResourceMode resMode, const ScnUI32 ammRenderSlots, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const optSrcProps, const void* optSrcData) {
    ScnBOOL r = ScnFALSE;
    STScnTextureOpq* opq = (STScnTextureOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(cfg != NULL && cfg->width > 0 && cfg->height > 0 && opq->cfg.width == 0){
        ScnBitmapRef bmp = ScnBitmap_alloc(opq->ctx);
        if(!ScnBitmap_create(bmp, cfg->width, cfg->height, cfg->color)){
            //error
        } else if(optSrcProps != NULL && optSrcData != NULL && -1 == ScnBitmap_pasteBitmapData(bmp, (STScnPoint2DI){ 0, 0 }, (STScnRectI){ 0, 0, optSrcProps->size.width, optSrcProps->size.height }, optSrcProps, optSrcData).width){
            //error
        } else {
            STScnTextureSlot* slots = ScnContext_malloc(opq->ctx, sizeof(STScnTextureSlot) * ammRenderSlots, SCN_DBG_STR("ScnTexture_prepare::slots"));
            if(slots != NULL){
                //slots (render)
                {
                    STScnTextureSlot* s = slots;
                    const STScnTextureSlot* sAfterEnd = s + ammRenderSlots;
                    while(s < sAfterEnd){
                        ScnTextureSlot_init(opq->ctx, s);
                        //force first full-sync
                        ScnChangesRngs_invalidateAll(&s->changes);
                        //
                        ++s;
                    }
                    opq->slots.arr = slots;
                    opq->slots.use = opq->slots.sz = ammRenderSlots;
                }
                //set
                ScnBitmap_set(&opq->bmp, bmp);
                opq->bmpProps   = ScnBitmap_getProps(bmp, NULL);
                //cfg
                opq->resMode    = resMode;
                opq->cfg        = *cfg;
                ScnGpuDevice_set(&opq->gpuDev, gpuDev);
                //ScnStruct_stRelease(ScnTextureCfg_getSharedStructMap(), &opq->cfg, sizeof(opq->cfg));
                //ScnStruct_stClone(ScnTextureCfg_getSharedStructMap(), cfg, sizeof(*cfg), &opq->cfg, sizeof(opq->cfg));
                //changes
                ScnChangesRngs_invalidateAll(&opq->changes);
                //
                r = ScnTRUE;
            }
        }
        ScnBitmap_release(&bmp);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//

STScnBitmapProps ScnTexture_getImageProps(ScnTextureRef ref, void** dstData){
    STScnBitmapProps r = STScnBitmapProps_Zero; void* data = NULL;
    STScnTextureOpq* opq = (STScnTextureOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        r = opq->bmpProps;
        if(!ScnBitmap_isNull(opq->bmp)){
            data = ScnBitmap_getData(opq->bmp);
        }
    }
    ScnMutex_unlock(opq->mutex);
    if(dstData != NULL) *dstData = data;
    return r;
}

void* ScnTexture_getImageData(ScnTextureRef ref, STScnBitmapProps* dstProps){
    void* r = NULL;
    STScnBitmapProps props = STScnBitmapProps_Zero;
    STScnTextureOpq* opq = (STScnTextureOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        props = opq->bmpProps;
        if(!ScnBitmap_isNull(opq->bmp)){
            r = ScnBitmap_getData(opq->bmp);
        }
    }
    ScnMutex_unlock(opq->mutex);
    if(dstProps != NULL) *dstProps = props;
    return r;
}

ScnBOOL ScnTexture_setImage(ScnTextureRef ref, const STScnBitmapProps* const srcProps, const void* srcData){
    ScnBOOL r = ScnFALSE;
    STScnTextureOpq* opq = (STScnTextureOpq*)ScnSharedPtr_getOpq(ref.ptr);
    if(!(srcProps != NULL && srcData != NULL)){
        return r;
    }
    ScnMutex_lock(opq->mutex);
    SCN_ASSERT(opq->resMode != ENScnResourceMode_Static || !ScnBitmap_isNull(opq->bmp)) //user logic error, static resource already synced
    if(opq->cfg.width == srcProps->size.width && opq->cfg.height == srcProps->size.height && !ScnBitmap_isNull(opq->bmp)){ //do not verify color here, let the 'ScnBitmap_pasteBitmapData' do it
        if(-1 == ScnBitmap_pasteBitmapData(opq->bmp, (STScnPoint2DI){ 0, 0 }, (STScnRectI){ 0, 0, srcProps->size.width, srcProps->size.height }, srcProps, srcData).width){
            //error
        } else {
            //changes
            ScnChangesRngs_invalidateAll(&opq->changes);
            //
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnTexture_setSubimage(ScnTextureRef ref, const STScnPoint2DI pos, const STScnBitmapProps* const srcProps, const void* srcData, const STScnRectI pSrcRect){
    ScnBOOL r = ScnFALSE;
    STScnTextureOpq* opq = (STScnTextureOpq*)ScnSharedPtr_getOpq(ref.ptr);
    if(!(srcProps != NULL && srcData != NULL)){
        return r;
    }
    ScnMutex_lock(opq->mutex);
    SCN_ASSERT(opq->resMode != ENScnResourceMode_Static || !ScnBitmap_isNull(opq->bmp)) //user logic error, static resource already synced
    if(!ScnBitmap_isNull(opq->bmp)){
        STScnRectI pastedRect = STScnRectI_Zero;
        if(opq->cfg.width <= 0 || opq->cfg.height <= 0){ //do not verify color here, let the 'ScnBitmap_pasteBitmapData' do it
            SCN_PRINTF_ERROR("ScnTexture_setSubimage, cfg.size not initialized.\n");
        } else if(-1 == (pastedRect = ScnBitmap_pasteBitmapData(opq->bmp, pos, pSrcRect, srcProps, srcData)).width){
            SCN_PRINTF_ERROR("ScnTexture_setSubimage, ScnBitmap_pasteBitmapData failed.\n");
        } else {
            SCN_ASSERT(pastedRect.width >= 0 && pastedRect.height >= 0)
            if(pastedRect.width > 0 && pastedRect.height > 0 && !opq->changes.all){
                const STScnRangeU rng = (STScnRangeU){ (ScnUI32)pastedRect.y, (ScnUI32)pastedRect.height };
                ScnChangesRngs_mergeRng(&opq->changes, &rng);
            }
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//gpu-buffer

ScnBOOL ScnTexture_prepareCurrentRenderSlot(ScnTextureRef ref){
    ScnBOOL r = ScnFALSE;
    STScnTextureOpq* opq = (STScnTextureOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->slots.arr != NULL && opq->slots.use > 0 && !ScnGpuDevice_isNull(opq->gpuDev)){
        if(ScnBitmap_isNull(opq->bmp)){
            r = (opq->resMode == ENScnResourceMode_Static ? ScnTRUE : ScnFALSE);
        } else {
            //sync gpu-buffer
            void* bmpData = NULL;
            STScnBitmapProps bmpProps = ScnBitmap_getProps(opq->bmp, &bmpData);
            //
            const ScnUI32 iFirst = opq->slots.iCur % opq->slots.use;
            STScnTextureSlot* slot = &opq->slots.arr[iFirst];
            //forward accumulated changes to all slots
            {
                ScnUI32 iSlot = iFirst;
                do {
                    STScnTextureSlot* slot = &opq->slots.arr[iSlot];
                    ScnChangesRngs_mergeWithOther(&slot->changes, &opq->changes);
                    //next
                    iSlot = (iSlot + 1) % opq->slots.use;
                } while(iSlot != iFirst);
                //reset accumulates changes
                ScnChangesRngs_reset(&opq->changes);
            }
            //
            if(ScnGpuTexture_isNull(slot->gpuTex)){
                slot->gpuTex = ScnGpuDevice_allocTexture(opq->gpuDev, &opq->cfg, &bmpProps, bmpData);
                if(!ScnGpuTexture_isNull(slot->gpuTex)){
                    r = ScnTRUE;
                }
            } else {
                STScnGpuTextureChanges changes = STScnGpuTextureChanges_Zero;
                changes.all = slot->changes.all;
                if(!changes.all){
                    changes.rngs = slot->changes.rngs.arr;
                    changes.rngsUse = slot->changes.rngs.use;
                }
                if(!changes.all && changes.rngsUse == 0){
                    //no changes to update
                    r = ScnTRUE;
                } else {
                    r = ScnGpuTexture_sync(slot->gpuTex, &opq->cfg, &bmpProps, bmpData, &changes);
                }
            }
            //destroy cpu-data (after first use)
            if(opq->resMode == ENScnResourceMode_Static){
                ScnBitmap_releaseAndNull(&opq->bmp);
            }
            //reset current slot changes
            ScnChangesRngs_reset(&slot->changes);
        }
    }
    ScnMutex_unlock(opq->mutex);
    //
    return r;
}

ScnBOOL ScnTexture_moveToNextRenderSlot(ScnTextureRef ref){
    ScnBOOL r = ScnFALSE;
    STScnTextureOpq* opq = (STScnTextureOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->slots.arr != NULL && opq->slots.use > 0 && !ScnGpuDevice_isNull(opq->gpuDev)){
        //move to next render slot
        opq->slots.iCur = (opq->slots.iCur + 1) % opq->slots.use;
        r = ScnTRUE;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnGpuTextureRef ScnTexture_getCurrentRenderSlot(ScnTextureRef ref){
    ScnGpuTextureRef r = ScnObjRef_Zero;
    STScnTextureOpq* opq = (STScnTextureOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->slots.arr != NULL && opq->slots.use > 0){
        STScnTextureSlot* slot = &opq->slots.arr[opq->slots.iCur % opq->slots.use];
        r = slot->gpuTex;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//STScnTextureSlot

void ScnTextureSlot_init(ScnContextRef ctx, STScnTextureSlot* opq){
    ScnMemory_setZeroSt(*opq);
    ScnContext_set(&opq->ctx, ctx);
    ScnChangesRngs_init(opq->ctx, &opq->changes);
}

void ScnTextureSlot_destroy(STScnTextureSlot* opq){
    ScnChangesRngs_destroy(&opq->changes);
    ScnGpuTexture_releaseAndNull(&opq->gpuTex);
    ScnContext_releaseAndNull(&opq->ctx);
}

//STScnRectU16

ScnBOOL ScnCompare_STScnRectU16(const ENScnCompareMode mode, const void* data1, const void* data2, const ScnUI32 dataSz){
    SCN_ASSERT(dataSz == sizeof(STScnRectU16))
    if(dataSz == sizeof(STScnRectU16)){
        const STScnRectU16* d1 = (STScnRectU16*)data1;
        const STScnRectU16* d2 = (STScnRectU16*)data2;
        switch (mode) {
            case ENScnCompareMode_Equal: return d1->y == d2->y && d1->x == d2->x;
            case ENScnCompareMode_Lower: return d1->y < d2->y || (d1->y == d2->y && d1->x < d2->x);
            case ENScnCompareMode_LowerOrEqual: return d1->y < d2->y || (d1->y == d2->y && d1->x <= d2->x);
            case ENScnCompareMode_Greater: return d1->y > d2->y || (d1->y == d2->y && d1->x > d2->x);
            case ENScnCompareMode_GreaterOrEqual: return d1->y > d2->y || (d1->y == d2->y && d1->x >= d2->x);
            default: SCN_ASSERT(ScnFALSE) break;
        }
    }
    return ScnFALSE;
}
