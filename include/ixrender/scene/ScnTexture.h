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
#include "ixrender/gpu/ScnGpuTexture.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnTextureRef

SCN_REF_STRUCT_METHODS_DEC(ScnTexture)

//

ScnBOOL ScnTexture_prepare(ScnTextureRef ref, const STScnGpuTextureCfg* cfg);
ScnBOOL ScnTexture_setImage(ScnTextureRef ref, const STScnBitmapProps srcProps, const ScnBYTE* srcData);
ScnBOOL ScnTexture_setSubimage(ScnTextureRef ref, const STScnPoint2DI pos, const STScnBitmapProps srcProps, const ScnBYTE* srcData, const STScnRectI srcRect);


#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnTexture_h */
