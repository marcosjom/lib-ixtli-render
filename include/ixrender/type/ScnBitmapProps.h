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
#include "ixrender/type/ScnBitmapColor.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#define STScnBitmapProps_Zero   { ENScnBitmapColor_undef, STScnSize2DU16_Zero, 0u, 0u }

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
