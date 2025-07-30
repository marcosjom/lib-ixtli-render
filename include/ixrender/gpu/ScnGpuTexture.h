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

#ifdef __cplusplus
extern "C" {
#endif

//ENScnGpuTextureIdx

typedef enum ENScnGpuTextureIdx_ {
    ENScnGpuTextureIdx_0 = 0,
    ENScnGpuTextureIdx_1,
    ENScnGpuTextureIdx_2,
    ENScnGpuTextureIdx_3,
    //Count
    ENScnGpuTextureIdx_Count
} ENScnGpuTextureIdx;

//ENScnGpuTextureCoordMode

typedef enum ENScnGpuTextureCoordMode_ {
    ENScnGpuTextureCoordMode_Repeat = 0, //pattern
    ENScnGpuTextureCoordMode_Clamp,      //single-image
    //
    ENScnGpuTextureCoordMode_Count
} ENScnGpuTextureCoordMode;

//ENScnGpuTexturePixelMode

typedef enum ENScnGpuTexturePixelMode_ {
    ENScnGpuTexturePixelMode_Nearest = 0, //fast selection of nearest color
    ENScnGpuTexturePixelMode_Linear,      //calculation of merged color
    //
    ENScnGpuTexturePixelMode_Count
} ENScnGpuTexturePixelMode;

//STScnGpuTextureCfg

#define STScnGpuTextureCfg_Zero   { ENScnBitmapColor_undef, 0, 0, ScnFALSE }

typedef struct STScnGpuTextureCfg_ {
    ENScnBitmapColor    color;
    ScnUI32             width;
    ScnUI32             height;
    ScnBOOL             mipmapEnabled;
    ENScnGpuTextureCoordMode coordMode;
    ENScnGpuTexturePixelMode pixelMode;
} STScnGpuTextureCfg;

//STScnGpuTextureChanges

typedef struct STScnGpuTextureChanges_ {
    ScnBOOL         whole;  //all the content is new
    STScnRectI*     rects;  //subimages areas
    ScnUI32         recsUse;
} STScnGpuTextureChanges;

//STScnGpuTextureApiItf

typedef struct STScnGpuTextureApiItf_ {
    void* (*create)(const STScnGpuTextureCfg* cfg, void* usrData);
    void  (*destroy)(void* data, void* usrData);
    //
    ScnBOOL  (*sync)(void* data, const STScnGpuTextureCfg* cfg, const STScnBitmapProps srcProps, const ScnBYTE* srcData, const STScnGpuTextureChanges* changes, void* usrData);
} STScnGpuTextureApiItf;

//ScnGpuTextureRef

SCN_REF_STRUCT_METHODS_DEC(ScnGpuTexture)

//

ScnBOOL ScnGpuTexture_prepare(ScnGpuTextureRef ref, const STScnGpuTextureCfg* cfg, const STScnGpuTextureApiItf* itf, void* itfParam);
ScnBOOL ScnGpuTexture_setImage(ScnGpuTextureRef ref, const STScnBitmapProps srcProps, const ScnBYTE* srcData);
ScnBOOL ScnGpuTexture_setSubimage(ScnGpuTextureRef ref, const STScnPointI pos, const STScnBitmapProps srcProps, const ScnBYTE* srcData, const STScnRectI srcRect);


#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuTexture_h */
