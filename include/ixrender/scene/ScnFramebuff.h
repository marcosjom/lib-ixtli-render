//
//  ScnFramebuff.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnFramebuff_h
#define ScnFramebuff_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/gpu/ScnGpuDevice.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnFramebuffRef

/** @struct ScnFramebuffRef
 *  @brief ScnFramebuff shared pointer. An object that defines a render destination.
 */

#define ScnFramebuffRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnFramebuff)

//

/**
 * @brief Prepares the abstract framebuffer with the provided gpu-device; containing one cpu-data slot and one or multiple gpu-data slots.
 * @param ref Reference to object.
 * @param gpuDev Gpu-device this buffer will be attached to.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnFramebuff_prepare(ScnFramebuffRef ref, ScnGpuDeviceRef gpuDev);

//binding

/**
 * @brief Binds the framebuffer with the provided opaque Operating System's View (MTKView for Apple, HWND for WIndows, etc...).
 * @param ref Reference to object.
 * @param osView Opaque pointer to eh  Operating System's View (MTKView for Apple, HWND for WIndows, etc...).
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnFramebuff_bindToOSView(ScnFramebuffRef ref, void* osView);

/**
 * @brief Retrieves the size of the frambuffer in pixels.
 * @param ref Reference to object.
 * @return Size in pixels on success, STScnSize2DU_Zero otherwise.
 */
STScnSize2DU ScnFramebuff_getSize(ScnFramebuffRef ref);

/**
 * @brief Sets of flags-for-sync the size of the frambuffer in pixels.
 * @note The internal action is API specific, parameters could be ignored.
 * @param ref Reference to object.
 * @param size Known current or future changed size.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnFramebuff_syncSize(ScnFramebuffRef ref, const STScnSize2DU size);

//

/**
 * @brief Retrieves the framebuffer properties.
 * @param ref Reference to object.
 * @return Framebuffer properties on success, STScnGpuFramebuffProps_Zero otherwise.
 */
STScnGpuFramebuffProps ScnFramebuff_getProps(ScnFramebuffRef ref);

/**
 * @brief Sets of flags-for-sync the frambuffer's properties.
 * @note The internal action is API specific, parameters could be ignored.
 * @param ref Reference to object.
 * @param props Known current or future changed properties.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnFramebuff_setProps(ScnFramebuffRef ref, const STScnGpuFramebuffProps* const props);

//gpu

/**
 * @brief Prepares the current render slot for gpu commands execution. The slot's gpu-data is synchronized with the current cpu-data.
 * @param ref Reference to object.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnFramebuff_prepareCurrentRenderSlot(ScnFramebuffRef ref);

/**
 * @brief Moves the index to the next render slot for future gpu commands execution.
 * @param ref Reference to object.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnFramebuff_moveToNextRenderSlot(ScnFramebuffRef ref);

/**
 * @brief Retrieves the current render slot gpu-data reference.
 * @param ref Reference to object.
 * @return ScnGpuFramebuff reference on success, ScnGpuFramebuffRef_Zero otherwise.
 */
ScnGpuFramebuffRef ScnFramebuff_getCurrentRenderSlot(ScnFramebuffRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnFramebuff_h */
