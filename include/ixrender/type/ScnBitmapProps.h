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

/** @enum ENScnBitmapColor
 *  @brief Colors used by this library.
 *  @var ENScnBitmapColor undef
 *  Undefined color, this value is zero.
 *  @var ENScnBitmapColor ALPHA8
 *  Alpha only, RGB is assumed to white.
 *  @var ENScnBitmapColor GRAY8
 *  Gray only, RGB are the same value, alpha is assumed to opaque.
 *  @var ENScnBitmapColor GRAYALPHA8
 *  Gray and alpha, RGB are the same value.
 *  @var ENScnBitmapColor RGB8
 *  RGB only, alpha is assumed to opaque.
 *  @var ENScnBitmapColor RGBA8
 *  RGBA components.
 *  @var ENScnBitmapColor Count
 *  Enum's boundary.
 */
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

//STScnBitmapProps

/** @struct STScnBitmapProps
 *  @brief Bitmap properties definition.
 *  @var STScnBitmapProps::color
 *  Bitmap's color.
 *  @var STScnBitmapProps::size
 *  Bitmap's size.
 *  @var STScnBitmapProps::bitsPerPx
 *  Bitmap's bits per pixel; multiple of 8.
 *  @var STScnBitmapProps::bytesPerLine
 *  Bitmap's bytes per vertical line; usually word aligned.
 */

#define STScnBitmapProps_Zero   { ENScnBitmapColor_undef, STScnSize2DU16_Zero, 0, 0 }

typedef struct STScnBitmapProps {
    ENScnBitmapColor    color;
    STScnSize2DU16      size;
    ScnUI16             bitsPerPx;
    ScnUI16             bytesPerLine;
} STScnBitmapProps;

/**
 * @brief Creates a valid STScnBitmapProps.
 * @param width Bitmap's width.
 * @param height Bitmap's height.
 * @param color Bitmap's color.
 * @return A valid STScnBitmapProps on success, STScnBitmapProps_Zero otherwise.
 */
STScnBitmapProps ScnBitmapProps_build(const ScnSI32 width, const ScnSI32 height, const ENScnBitmapColor color);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnBitmapProps_h */
