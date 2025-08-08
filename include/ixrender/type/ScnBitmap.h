//
//  ScnBitmap.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnBitmap_h
#define ScnBitmap_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/type/ScnPoint.h"
#include "ixrender/type/ScnSize.h"
#include "ixrender/type/ScnRect.h"
#include "ixrender/type/ScnBitmapProps.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnBitmap

typedef struct STScnBitmap {
    STScnBitmapProps    props;
    ScnBYTE*            data;
    ScnUI32*            dataSz;
} STScnBitmap;

//ScnBitmapRef

#define ScnBitmapRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnBitmap)

ScnBOOL ScnBitmap_create(ScnBitmapRef ref, const ScnSI32 width, const ScnSI32 height, const ENScnBitmapColor color);
ScnBOOL ScnBitmap_pasteBitmapData(ScnBitmapRef ref, const STScnPoint2DI dstPos, const STScnRectI srcRect, const STScnBitmapProps* const srcProps, const void* srcData);
//
STScnBitmapProps    ScnBitmap_getProps(ScnBitmapRef ref, void** optDstData);
void*               ScnBitmap_getData(ScnBitmapRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnBitmap_h */
