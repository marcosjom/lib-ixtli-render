//
//  ScnApiMetalTexture.m
//  ixtli-render
//
//  Created by Marcos Ortega on 18/8/25.
//

#include "ScnApiMetalTexture.h"

void ScnApiMetalTexture_free(void* pObj){
    STScnApiMetalTexture* obj = (STScnApiMetalTexture*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        if(obj->tex != nil){
            [obj->tex release];
            obj->tex = nil;
        }
        ScnGpuSampler_releaseAndNull(&obj->sampler);
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

ScnBOOL ScnApiMetalTexture_sync(void* pObj, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData, const STScnGpuTextureChanges* const changes){
    ScnBOOL r = ScnFALSE;
    STScnApiMetalTexture* obj = (STScnApiMetalTexture*)pObj;
    if(!(obj != NULL && obj->tex != NULL && srcProps != NULL && srcData != NULL && changes != NULL)){
        //missing params or objects
        return r;
    }
    if(!(obj->cfg.width == cfg->width && obj->cfg.height == cfg->height && obj->cfg.color == cfg->color)){
        //change of size of color is not supported
        return r;
    }
    //sync
    {
        ScnUI32 pxUpdated = 0, lnsUpdated = 0;
        if(changes->all){
            //update the texture
            MTLRegion region = { { 0, 0, 0 }, {obj->cfg.width, obj->cfg.height, 1} };
            [obj->tex replaceRegion:region mipmapLevel:0 withBytes:srcData bytesPerRow:srcProps->bytesPerLine];
            pxUpdated   += (obj->cfg.width * obj->cfg.height);
            lnsUpdated  += obj->cfg.height;
            r = ScnTRUE;
        } else if(changes->rngsUse == 0){
            //nothing to sync
            r = ScnTRUE;
        } else if(changes->rngs != 0){
#           ifdef SCN_ASSERTS_ACTIVATED
            ScnUI32 prevRngAfterEnd = 0;
#           endif
            const STScnRangeU* rng = changes->rngs;
            const STScnRangeU* rngAfterEnd = rng + changes->rngsUse;
            r = ScnTRUE;
            while(rng < rngAfterEnd){
                SCN_ASSERT(prevRngAfterEnd <= rng->start + rng->size) //rngs should be ordered and non-overlapping
                if(rng->start > obj->cfg.height || (rng->start + rng->size) > obj->cfg.height){
                    //out of range
                    r = ScnFALSE;
                    break;
                }
                //update the texture area
                {
                    MTLRegion region = { { 0, rng->start, 0 }, {obj->cfg.width, rng->size, 1} };
                    const void* srcRow = ((ScnBYTE*)srcData) + (rng->start * srcProps->bytesPerLine);
                    [obj->tex replaceRegion:region mipmapLevel:0 withBytes:srcRow bytesPerRow:srcProps->bytesPerLine];
                    pxUpdated   += (obj->cfg.width * rng->size);
                    lnsUpdated  += rng->size;
                }
#               ifdef SCN_ASSERTS_ACTIVATED
                prevRngAfterEnd = rng->start + rng->size;
#               endif
                ++rng;
            }
            r = ScnTRUE;
        }
        SCN_PRINTF_VERB("%2.f%% %u of %u lines synced at texture (%.1f Kpixs, %.1f KBs).\n", (float)lnsUpdated * 100.f / (float)obj->cfg.height, lnsUpdated, obj->cfg.height, (ScnFLOAT)pxUpdated / 1024.f, (ScnFLOAT)lnsUpdated * (ScnFLOAT)srcProps->bytesPerLine / 1024.f);
    }
    return r;
}
