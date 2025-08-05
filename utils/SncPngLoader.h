//
//  ScnPngLoader.h
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/11/18.
//

#ifndef ScnPngLoader_h
#define ScnPngLoader_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/type/ScnBitmap.h"

#ifdef __cplusplus
extern "C" {
#endif

ScnBOOL ScnPngLoader_loadFromPath(ScnContextRef ctx, const char* path, STScnBitmapProps* dstProps, void** dstData, ScnUI32* dstDataSz);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnPngLoader_h */
