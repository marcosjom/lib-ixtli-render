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

//STScnTextureRef

SCN_REF_STRUCT_METHODS_DEC(ScnTexture)

//

ScnBOOL ScnTexture_prepare(STScnTextureRef ref, const STScnGpuTextureCfg* cfg);
ScnBOOL ScnTexture_setImage(STScnTextureRef ref, const STScnBitmapProps srcProps, const ScnBYTE* srcData);
ScnBOOL ScnTexture_setSubimage(STScnTextureRef ref, const STScnPointI pos, const STScnBitmapProps srcProps, const ScnBYTE* srcData, const STScnRectI srcRect);


#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnTexture_h */
