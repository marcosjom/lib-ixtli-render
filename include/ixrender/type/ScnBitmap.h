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

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ENScnBitmapColor_ {
    ENScnBitmapColor_undef = 0,
    ENScnBitmapColor_ALPHA8,        //only alpha (8 bits)
    ENScnBitmapColor_GRIS8,        //grayscale (8 bits)
    ENScnBitmapColor_GRISALPHA8,    //grayscale and alpha (8 bits each component)
    ENScnBitmapColor_RGB4,        //RGB (4 bits each component)
    ENScnBitmapColor_RGB8,        //RGB (8 bits each component)
    ENScnBitmapColor_RGBA4,        //RGBA (4 bits each component)
    ENScnBitmapColor_RGBA8,        //RGBA (8 bits each component)
    ENScnBitmapColor_ARGB4,        //ARGB (4 bits each component)
    ENScnBitmapColor_ARGB8,        //ARGB (8 bits each component)
    ENScnBitmapColor_BGRA8,        //BGRA (8 bits each component)
    ENScnBitmapColor_SWF_PIX15,    //
    ENScnBitmapColor_SWF_PIX24,    //reserved+R+G+B
    ENScnBitmapColor_Count
} ENScnBitmapColor;

#define STScnBitmapProps_Zero   { ENScnBitmapColor_undef, STScnSizeI_Zero, 0, 0 }

typedef struct STScnBitmapProps_ {
    ENScnBitmapColor    color;
    STScnSizeI          size;
    ScnSI32             bitsPerPx;
    ScnSI32             bytesPerLine;
} STScnBitmapProps;

STScnBitmapProps ScnBitmapProps_build(const ScnSI32 width, const ScnSI32 height, const ENScnBitmapColor color);

//STScnBitmap

typedef struct STScnBitmap_ {
    STScnBitmapProps    props;
    ScnBYTE*            data;
    ScnUI32*            dataSz;
} STScnBitmap;

//STScnBitmapRef

SCN_REF_STRUCT_METHODS_DEC(ScnBitmap)

ScnBOOL ScnBitmap_create(STScnBitmapRef ref, const ScnSI32 width, const ScnSI32 height, const ENScnBitmapColor color);
ScnBOOL ScnBitmap_pasteBitmapData(STScnBitmapRef ref, const STScnPointI pos, const STScnBitmapProps srcProps, const ScnBYTE* srcData);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnBitmap_h */
