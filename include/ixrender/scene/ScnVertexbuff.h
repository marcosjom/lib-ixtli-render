//
//  ScnVertexbuff.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnVertexbuff_h
#define ScnVertexbuff_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/scene/ScnBuffer.h"
#include "ixrender/gpu/ScnGpuDevice.h"
#include "ixrender/gpu/ScnGpuVertexbuff.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnVertexbuffRef

/** @struct ScnVertexbuffRef
 *  @brief ScnVertexbuff shared pointer. An object that stores vertices and indices buffers for the drawing commands.
 */

#define ScnVertexbuffRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnVertexbuff)

//

/**
 * @brief Prepares the abstract vertexbuffer with the provided gpu-device; containing one cpu-data slot and one or multiple gpu-data slots.
 * @param ref Reference to object.
 * @param gpuDev Gpu-device this buffer will be attached to.
 * @param ammRenderSlots Ammount of render slots required, if the resource is dynamic.
 * @param cfg Vertexbuffer configuration.
 * @param vertexBuff Buffer for vertices data.
 * @param idxsBuff Buffer for indices data.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuff_prepare(ScnVertexbuffRef ref, ScnGpuDeviceRef gpuDev, const ScnUI32 ammRenderSlots, const STScnGpuVertexbuffCfg* const cfg, ScnBufferRef vertexBuff, ScnBufferRef idxsBuff);

/**
 * @brief Retrieves the size per vertex's record in bytes.
 * @param ref Reference to object.
 * @return Size in bytes on success, zero otherwise.
 */
ScnUI32 ScnVertexbuff_getSzPerRecord(ScnVertexbuffRef ref);

/**
 * @brief Retrieves the size per vertex-index's record in bytes.
 * @param ref Reference to object.
 * @return Size in bytes on success, zero otherwise.
 */
ScnUI32 ScnVertexbuff_getSzPerIndex(ScnVertexbuffRef ref);

/**
 * @brief Retrieves the buffer storing the vertices' data.
 * @param ref Reference to object.
 * @return Buffer's reference on success, ScnBufferRef_Zero otherwise.
 */
ScnBufferRef ScnVertexbuff_getVertexBuff(ScnVertexbuffRef ref);

/**
 * @brief Retrieves the buffer storing the vertex indices' data.
 * @param ref Reference to object.
 * @return Buffer's reference on success, ScnBufferRef_Zero otherwise.
 */
ScnBufferRef ScnVertexbuff_getIdxsBuff(ScnVertexbuffRef ref);

//gpu-vertexbuffer

/**
 * @brief Prepares the current render slot for gpu commands execution. The slot's gpu-data is synchronized with the current cpu-data.
 * @param ref Reference to object.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */

ScnBOOL ScnVertexbuff_prepareCurrentRenderSlot(ScnVertexbuffRef ref);

/**
 * @brief Moves the index to the next render slot for future gpu commands execution.
 * @param ref Reference to object.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuff_moveToNextRenderSlot(ScnVertexbuffRef ref);

/**
 * @brief Retrieves the current render slot gpu-data reference.
 * @param ref Reference to object.
 * @return ScnGpuVertexbuff reference on success, ScnGpuVertexbuffRef_Zero otherwise.
 */
ScnGpuVertexbuffRef ScnVertexbuff_getCurrentRenderSlot(ScnVertexbuffRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnVertexbuff_h */
