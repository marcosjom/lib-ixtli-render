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

/** @struct STScnBitmap
 *  @brief Image bitmap.
 *  @var STScnBitmap::props
 *  Bitmap description.
 *  @var STScnBitmap::data
 *  Bitmap buffer containing the payload.
 *  @var STScnBitmap::dataSz
 *  Bitmap buffer's size in bytes.
 */

typedef struct STScnBitmap {
    STScnBitmapProps    props;
    ScnBYTE*            data;
    ScnUI32*            dataSz;
} STScnBitmap;

//ScnBitmapRef

/** @struct ScnBitmapRef
 *  @brief ScnBitmap shared pointer. Provides bitmap creation and manipulation functionalities.
 */

#define ScnBitmapRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnBitmap)

/**
 * @brief Creates or reuses an internal buffer for the required bitmap.
 * @param ref Reference to object.
 * @param width Bitmap width size.
 * @param height Bitmap height size.
 * @param color Bitmap color format.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnBitmap_create(ScnBitmapRef ref, const ScnSI32 width, const ScnSI32 height, const ENScnBitmapColor color);

/**
 * @brief Pastes an external bitmap into the internal buffer.
 * @param ref Reference to object.
 * @param dstPos Paste top-left position in internal bitmap coordinates.
 * @param srcRect Copy rectangle in external bitmap coordinates.
 * @param srcProps External bitmap properties.
 * @param srcData External bitmap buffer.
 * @return The internal buffer affected STScnRectI, or { 0, 0, -1, -1 } if params are not valid or any error.
 */
STScnRectI ScnBitmap_pasteBitmapData(ScnBitmapRef ref, const STScnPoint2DI dstPos, const STScnRectI srcRect, const STScnBitmapProps* const srcProps, const void* srcData); //returns { 0, 0, -1, -1 } on error

/**
 * @brief Retrieves the bitmap properties.
 * @param ref Reference to object.
 * @param optDstData Destination of internal buffer pointer, optional.
 * @return A STScnBitmapProps on success, STScnBitmapProps_Zero otherwise.
 */
STScnBitmapProps ScnBitmap_getProps(ScnBitmapRef ref, void** optDstData);

/**
 * @brief Retrieves the bitmap internal buffer.
 * @param ref Reference to object.
 * @return The internal buffer pointer on success, NULL otherwise.
 */
void* ScnBitmap_getData(ScnBitmapRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnBitmap_h */
