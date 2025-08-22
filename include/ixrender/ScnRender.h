//
//  ScnRender.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnRender_h
#define ScnRender_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/ScnRenderCfg.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/core/ScnCompare.h"
#include "ixrender/type/ScnRange.h"
#include "ixrender/gpu/ScnGpuBuffer.h"
#include "ixrender/gpu/ScnGpuVertexbuff.h"
#include "ixrender/gpu/ScnGpuTexture.h"
#include "ixrender/gpu/ScnGpuRenderbuff.h"
#include "ixrender/gpu/ScnGpuFramebuff.h"
#include "ixrender/gpu/ScnGpuDevice.h"
#include "ixrender/api/ScnApiItf.h"
#include "ixrender/scene/ScnResourceMode.h"
#include "ixrender/scene/ScnTransform2d.h"
#include "ixrender/scene/ScnBuffer.h"
#include "ixrender/scene/ScnVertexbuffs.h"
#include "ixrender/scene/ScnFramebuff.h"
#include "ixrender/scene/ScnRenderCmd.h"
#include "ixrender/scene/ScnNode2d.h"
#include "ixrender/scene/ScnModel2d.h"
#include "ixrender/scene/ScnRenderJob.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnRenderRef

/** @struct ScnRenderRef
 *  @brief ScnRender shared pointer. This is the top object in this library, used as factory and render engine.
 */

#define ScnRenderRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnRender)

/**
 * @brief Provides the default configuration.
 * @note Users should retrieve this configuration and updated its member for customization.
 * @return The default configurations with all its members populated.
 */
STScnRenderCfg ScnRender_getDefaultCfg(void);

/**
 * @brief Prepares the render object with the povided API and configuration.
 * @param ref Reference to object.
 * @param itf API interface methods structure.
 * @param itfParam API interface param to be passed when calling the interface methods.
 * @param optCfg Render configuration, optional.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRender_prepare(ScnRenderRef ref, const STScnApiItf* itf, void* itfParam, const STScnRenderCfg* optCfg);

/**
 * @brief Selects and opens a device for rendering based on the configuration provided.
 * @param ref Reference to object.
 * @param cfg Device configuration for device selection.
 * @param ammRenderSlots Ammount of render slots to be created.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRender_openDevice(ScnRenderRef ref, const STScnGpuDeviceCfg* cfg, const ScnUI32 ammRenderSlots);

/**
 * @brief Opened device verification.
 * @param ref Reference to object.
 * @return ScnTRUE if a device is open, ScnFALSE otherwise.
 */
ScnBOOL ScnRender_hasOpenDevice(ScnRenderRef ref);

/**
 * @brief Open device opaque pointer retrieval.
 * @param ref Reference to object.
 * @return non-null opaque pointer if device is open, NULL otherwise.
 */
void* ScnRender_getApiDevice(ScnRenderRef ref);

/**
 * @brief Open device description retrieval.
 * @param ref Reference to object.
 * @return A device description if a device is open, STScnGpuDeviceDesc_Zero otherwise.
 */

STScnGpuDeviceDesc ScnRender_getDeviceDesc(ScnRenderRef ref);

//model

/**
 * @brief Allocates a ScnModel2d object, linked to the ScnRender current ScnContext.
 * @param ref Reference to object.
 * @return A ScnModel2d shared pointer on success, ScnModel2dRef_Zero otherwise.
 */
ScnModel2dRef ScnRender_allocModel(ScnRenderRef ref);

//framebuffer

/**
 * @brief Allocates a ScnFramebuff object, linked to the ScnRender current ScnContext.
 * @param ref Reference to object.
 * @return A ScnFramebuff shared pointer on success, ScnFramebuffRef_Zero otherwise.
 */
ScnFramebuffRef ScnRender_allocFramebuff(ScnRenderRef ref);

//texture

/**
 * @brief Allocates a ScnTexture object, linked to the ScnRender current ScnContext.
 * @param ref Reference to object.
 * @param mode Resource mode; defines how its internal render-slots behaves.
 * @param cfg Texture configuration.
 * @param optSrcProps Texture initial data Bitmap configuration, optional.
 * @param optSrcData Texture initial data Bitmap payload, optional.
 * @return A ScnTexture shared pointer on success, ScnTextureRef_Zero otherwise.
 */
ScnTextureRef ScnRender_allocTexture(ScnRenderRef ref, const ENScnResourceMode mode, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const optSrcProps, const void* optSrcData);

//sampler

/**
 * @brief Allocates a ScnGpuSampler object, linked to the ScnRender current ScnContext.
 * @param ref Reference to object.
 * @param cfg Sampler configuration.
 * @return A ScnGpuSampler shared pointer on success, ScnGpuSamplerRef_Zero otherwise.
 */
ScnGpuSamplerRef ScnRender_allocSampler(ScnRenderRef ref, const STScnGpuSamplerCfg* const cfg);

//renderJob

/**
 * @brief Allocates a ScnRenderJob object if a non-busy render slot is available.
 * @param ref Reference to object.
 * @return A ScnRenderJob shared pointer on success, ScnRenderJobRef_Zero otherwise.
 */
ScnRenderJobRef ScnRender_allocRenderJob(ScnRenderRef ref); //if render slot is available

/**
 * @brief If a non-busy render slot is available, syncs the gpu-data with the current cpu-data, builds the gpu render job and starts the job's execution on gpu.
 * @note The internal render job's data is swaped with a copy in the render-slot to avoid the need of copying it. This means a render job cannot be re-used, and has to be rebuilt even if the scene is exactly the same.
 * @note A render job retains the objects used in the scene (like textures, buffers and samplers, ...); the render job is cleared (and these objects released) when this job is completed and a new job needs to use the same render-slot.
 * @param ref Reference to object.
 * @param job Render job to execute.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRender_enqueue(ScnRenderRef ref, ScnRenderJobRef job);

//custom actions (for custom shaders)

/**
 * @brief Retrieves the default ScnVertexbuffs object.
 * @note Users can (and should) allocate their own ScnVertexbuffs for big models. The default ScnVertexbuffs is intended for small models.
 * @param ref Reference to object.
 * @return A ScnVertexbuffs shared pointer on success,  ScnVertexbuffsRef_Zero otherwise.
 */
ScnVertexbuffsRef ScnRender_getDefaultVertexbuffs(ScnRenderRef ref);

/**
 * @brief Allocates a ScnVertexbuffs object, linked to the ScnRender current ScnContext.
 * @note Users can (and should) allocate their own ScnVertexbuffs for big models. The default ScnVertexbuffs is intended for small models.
 * @param ref Reference to object.
 * @return A ScnVertexbuffs shared pointer on success,  ScnVertexbuffsRef_Zero otherwise.
 */
ScnVertexbuffsRef ScnRender_allocVertexbuffs(ScnRenderRef ref);

/**
 * @brief Allocates a ScnBuffer object, linked to the ScnRender current ScnContext.
 * @param ref Reference to object.
 * @return A ScnBuffer shared pointer on success,  ScnBufferRef_Zero otherwise.
 */
ScnBufferRef ScnRender_allocBuffer(ScnRenderRef ref, const STScnGpuBufferCfg* const cfg);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRender_h */
