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
#include "ixrender/gpu/ScnGpuDevice.h"
#include "ixrender/gpu/ScnGpuTexture.h"
#include "ixrender/scene/ScnResourceMode.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnTextureRef

/** @struct ScnTextureRef
 *  @brief ScnTexture shared pointer. An object that contains one cpu-texture and one-or-more gpu-textures (for the render slots).
 */

#define ScnTextureRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnTexture)

//

/**
 * @brief Prepares the abstract texture with the provided gpu-device; containing one cpu-data slot and one or multiple gpu-data slots.
 * @param ref Reference to object.
 * @param gpuDev Gpu-device this buffer will be attached to.
 * @param resMode Resource mode to determine if the cpu-data must be released after use, and how many gpu-data slots will be created.
 * @param ammRenderSlots Ammount of render slots required, if the resource is dynamic.
 * @param cfg Texture configuration.
 * @param optSrcProps Texture initial content properties, optional.
 * @param optSrcData Texture initial content pointer, optional.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnTexture_prepare(ScnTextureRef ref, ScnGpuDeviceRef gpuDev, const ENScnResourceMode resMode, const ScnUI32 ammRenderSlots, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const optSrcProps, const void* optSrcData);

//

/**
 * @brief Updates the texture's whole content.
 * @param ref Reference to object.
 * @param srcProps Texture's content properties.
 * @param srcData Texture's content pointer.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnTexture_setImage(ScnTextureRef ref, const STScnBitmapProps* const srcProps, const void* srcData);

/**
 * @brief Updates a portion of the texture's content. The affected lines are flagged for synchronization in all gpu-slots.
 * @param ref Reference to object.
 * @param pos Top-left position for the new content in the texture.
 * @param srcProps Content's source properties.
 * @param srcData Content's source pointer.
 * @param srcRect Content's source area.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnTexture_setSubimage(ScnTextureRef ref, const STScnPoint2DI pos, const STScnBitmapProps* const srcProps, const void* srcData, const STScnRectI srcRect);

/**
 * @brief Retrieves the texture properties.
 * @param ref Reference to object.
 * @return The texture properties on success, STScnBitmapProps_Zero otherwise.
 */
STScnBitmapProps ScnTexture_getImageProps(ScnTextureRef ref, void** dstData);

/**
 * @brief Retrieves the texture cpu-data pointer.
 * @param ref Reference to object.
 * @return The texture's cpu-data pointer on success, NULL otherwise.
 */
void* ScnTexture_getImageData(ScnTextureRef ref, STScnBitmapProps* dstProps);

//gpu-buffer

/**
 * @brief Prepares the current render slot for gpu commands execution. The slot's gpu-data is synchronized with the current cpu-data.
 * @param ref Reference to object.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnTexture_prepareCurrentRenderSlot(ScnTextureRef ref);

/**
 * @brief Moves the index to the next render slot for future gpu commands execution.
 * @param ref Reference to object.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnTexture_moveToNextRenderSlot(ScnTextureRef ref);

/**
 * @brief Retrieves the current render slot gpu-data reference.
 * @param ref Reference to object.
 * @return ScnGpuTexture reference on success, ScnGpuTextureRef_Zero otherwise.
 */
ScnGpuTextureRef ScnTexture_getCurrentRenderSlot(ScnTextureRef ref);


#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnTexture_h */
