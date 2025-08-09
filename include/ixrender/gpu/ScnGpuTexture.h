//
//  ScnGpuTexture.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnGpuTexture_h
#define ScnGpuTexture_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/type/ScnPoint.h"
#include "ixrender/type/ScnRect.h"
#include "ixrender/type/ScnBitmap.h"
#include "ixrender/gpu/ScnGpuSampler.h"

#ifdef __cplusplus
extern "C" {
#endif

//ENScnGpuTextureIdx

typedef enum ENScnGpuTextureIdx {
    ENScnGpuTextureIdx_0 = 0,
    ENScnGpuTextureIdx_1,
    ENScnGpuTextureIdx_2,
    //Count
    ENScnGpuTextureIdx_Count
} ENScnGpuTextureIdx;

//STScnGpuTextureCfg

#define STScnGpuTextureCfg_Zero   { ENScnBitmapColor_undef, 0, 0, ScnFALSE, STScnGpuSamplerCfg_Zero }

typedef struct STScnGpuTextureCfg {
    ENScnBitmapColor    color;
    ScnUI32             width;
    ScnUI32             height;
    ScnBOOL             mipmapEnabled;
    STScnGpuSamplerCfg  sampler;
} STScnGpuTextureCfg;

//STScnGpuTextureChanges

#define STScnGpuTextureChanges_Zero {  ScnFALSE, NULL, 0 }

typedef struct STScnGpuTextureChanges {
    ScnBOOL         all;    //all the content is new
    STScnRectU16*   rects;  //subimages areas
    ScnUI32         recsUse;
} STScnGpuTextureChanges;

//STScnGpuTextureApiItf

typedef struct STScnGpuTextureApiItf {
    void        (*free)(void* data);
    //
    ScnBOOL     (*sync)(void* data, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData, const STScnGpuTextureChanges* const changes);
} STScnGpuTextureApiItf;

//ScnGpuTextureRef

#define ScnGpuTextureRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnGpuTexture)

//

ScnBOOL     ScnGpuTexture_prepare(ScnGpuTextureRef ref, const STScnGpuTextureApiItf* itf, void* itfParam);
void*       ScnGpuTexture_getApiItfParam(ScnGpuTextureRef ref);
//
ScnBOOL     ScnGpuTexture_sync(ScnGpuTextureRef ref, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData, const STScnGpuTextureChanges* const changes);


#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuTexture_h */
