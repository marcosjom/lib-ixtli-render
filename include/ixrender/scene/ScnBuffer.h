//
//  ScnBuffer.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnBuffer_h
#define ScnBuffer_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/core/ScnMemElastic.h"
#include "ixrender/type/ScnRange.h"
#include "ixrender/scene/ScnVertex.h"
#include "ixrender/gpu/ScnGpuDevice.h"
#include "ixrender/gpu/ScnGpuBuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnBufferRef

/** @struct ScnBufferRef
 *  @brief ScnBuffer shared pointer. An object that stores any payload to be synced-to and used-by the gpu.
 */

#define ScnBufferRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnBuffer)

//

/**
 * @brief Prepares the abstract buffer with the provided configuration and gpu-device; containing one cpu-data slot and one or multiple gpu-data slots.
 * @param ref Reference to object.
 * @param gpuDev Gpu-device this buffer will be attached to.
 * @param ammRenderSlots Ammount of gpu render slots to be internally created.
 * @param cfg Buffer's configuration.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnBuffer_prepare(ScnBufferRef ref, ScnGpuDeviceRef gpuDev, const ScnUI32 ammRenderSlots, const STScnGpuBufferCfg* const cfg);

/**
 * @brief Determines if the buffer has pointers assigned for usage in the cpu-slot.
 * @param ref Reference to object.
 * @return ScnTRUE if has pointers assigned, ScnFALSE otherwise.
 */
ScnBOOL ScnBuffer_hasPtrs(ScnBufferRef ref); //allocations made?

/**
 * @brief Retrieves the ammount of render slots (gpu-data) the buffer has.
 * @param ref Reference to object.
 * @return The ammount of render slots, zero otherwise.
 */
ScnUI32 ScnBuffer_getRenderSlotsCount(ScnBufferRef ref);

//cpu-buffer

/**
 * @brief Clears the assigned-pointers' index.
 * @param ref Reference to object.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnBuffer_clear(ScnBufferRef ref);

/**
 * @brief Flags the whole buffer as requiring synchronization. All gpu-slots will be synced before rendering.
 * @param ref Reference to object.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnBuffer_invalidateAll(ScnBufferRef ref);

/**
 * @brief Assigns a pointer to the required memory block size.
 * @param ref Reference to object.
 * @param usableSz Requested memory block's size in bytes.
 * @return A pointer on success, STScnAbsPtr_Zero otherwise.
 */
STScnAbsPtr ScnBuffer_malloc(ScnBufferRef ref, const ScnUI32 usableSz);

/**
 * @brief Frees a previously assigned memory block.
 * @param ref Reference to object.
 * @param ptr Pointer to previously assigned memory block.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnBuffer_mfree(ScnBufferRef ref, const STScnAbsPtr ptr);

/**
 * @brief Flags a previously assigned memory block's area for synchronization on all render slots.
 * @note The ranges for synchorinzations accumulate until the render slot is used.
 * @param ref Reference to object.
 * @param ptr Pointer inside a previously assigned memory block.
 * @param sz Size of the range that requires synchornization.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnBuffer_mInvalidate(ScnBufferRef ref, const STScnAbsPtr ptr, const ScnUI32 sz);

//gpu-buffer

/**
 * @brief Prepares the current render slot for gpu commands execution. The slot's gpu-data is synchronized with the current cpu-data.
 * @param ref Reference to object.
 * @param optDstHasPtrs Destination for flag set to ScnTRUE if the buffer had pointers assigned (explicit data).
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnBuffer_prepareCurrentRenderSlot(ScnBufferRef ref, ScnBOOL* optDstHasPtrs);

/**
 * @brief Moves the index to the next render slot for future gpu commands execution.
 * @param ref Reference to object.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnBuffer_moveToNextRenderSlot(ScnBufferRef ref);

/**
 * @brief Retrieves the current render slot gpu-data reference.
 * @param ref Reference to object.
 * @return ScnGpuBuffer reference on success, ScnGpuBufferRef_Zero otherwise.
 */
ScnGpuBufferRef ScnBuffer_getCurrentRenderSlot(ScnBufferRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnBuffer_h */
