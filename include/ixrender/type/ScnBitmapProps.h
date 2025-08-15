//
//  ScnBitmapProps.h
//  ixtli-render
//
//  Created by Marcos Ortega on 8/8/25.
//

#ifndef ScnBitmapProps_h
#define ScnBitmapProps_h

#include "ixrender/ixtli-defs.h"
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

#define STScnBitmapProps_Zero   { ENScnBitmapColor_undef, STScnSize2DU16_Zero, 0, 0 }

typedef struct STScnBitmapProps {
    ENScnBitmapColor    color;
    STScnSize2DU16      size;
    ScnUI16             bitsPerPx;
    ScnUI16             bytesPerLine;
} STScnBitmapProps;

STScnBitmapProps ScnBitmapProps_build(const ScnSI32 width, const ScnSI32 height, const ENScnBitmapColor color);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnBitmapProps_h */
