//
//  ScnTexture.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnTexture_h
#define ScnTexture_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/type/ScnPoint.h"
#include "ixrender/type/ScnRect.h"
#include "ixrender/type/ScnBitmap.h"
#include "ixrender/gpu/ScnGpuDevice.h"
#include "ixrender/gpu/ScnGpuTexture.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnTextureRef

SCN_REF_STRUCT_METHODS_DEC(ScnTexture)

//

ScnBOOL         ScnTexture_prepare(ScnTextureRef ref, ScnGpuDeviceRef gpuDev, const ScnUI32 ammRenderSlots, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const optSrcProps, const void* optSrcData);

//

ScnBOOL         ScnTexture_setImage(ScnTextureRef ref, const STScnBitmapProps* const srcProps, const void* srcData);
ScnBOOL         ScnTexture_setSubimage(ScnTextureRef ref, const STScnPoint2DI pos, const STScnBitmapProps* const srcProps, const void* srcData, const STScnRectI srcRect);

//gpu-buffer
ScnBOOL         ScnTexture_prepareCurrentRenderSlot(ScnTextureRef ref);
ScnBOOL         ScnTexture_moveToNextRenderSlot(ScnTextureRef ref);
ScnGpuTextureRef ScnTexture_getCurrentRenderSlot(ScnTextureRef ref);


#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnTexture_h */
