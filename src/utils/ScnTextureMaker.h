//
//  ScnTextureMaker.h
//  ixtli-render
//
//  Created by Marcos Ortega on 20/8/25.
//

#ifndef ScnTextureMaker_h
#define ScnTextureMaker_h

#include "ixrender/type/ScnBitmap.h"
#include "ixrender/core/ScnContext.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates a bitmap with a grid of colors, to be used as texture in demos.
 * @param ctx Reference to context.
 * @param width Texture width in pixels.
 * @param height Texture height in pixels.
 * @param gridSz Texture grid's blocks width and height in pixels.
 * @param color Texture color.
 * @return A bitmap on success, ScnBitmapRef_Zero otherwise.
 */
ScnBitmapRef ScnTextureMaker_make(ScnContextRef ctx, const ScnUI16 width, const ScnUI16 height, const ENScnBitmapColor color, const ScnUI16 gridSz);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnTextureMaker_h */
