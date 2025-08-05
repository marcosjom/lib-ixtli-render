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

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ENScnBitmapColor {
    ENScnBitmapColor_undef = 0,
    //
    ENScnBitmapColor_ALPHA8,        //only alpha (8 bits)
    ENScnBitmapColor_GRAY8,         //grayscale (8 bits)
    ENScnBitmapColor_GRAYALPHA8,    //grayscale and alpha (8 bits each component)
    ENScnBitmapColor_RGB8,          //RGB (8 bits each component)
    ENScnBitmapColor_RGBA8,         //RGBA (8 bits each component)
    //
    ENScnBitmapColor_Count
} ENScnBitmapColor;

#define STScnBitmapProps_Zero   { ENScnBitmapColor_undef, STScnSize2DI_Zero, 0, 0 }

typedef struct STScnBitmapProps {
    ENScnBitmapColor    color;
    STScnSize2DI        size;
    ScnSI32             bitsPerPx;
    ScnSI32             bytesPerLine;
} STScnBitmapProps;

STScnBitmapProps ScnBitmapProps_build(const ScnSI32 width, const ScnSI32 height, const ENScnBitmapColor color);

//STScnBitmap

typedef struct STScnBitmap {
    STScnBitmapProps    props;
    ScnBYTE*            data;
    ScnUI32*            dataSz;
} STScnBitmap;

//ScnBitmapRef

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
