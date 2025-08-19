//
//  ScnGpuDevice.h
//  ixtli-render
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnGpuDevice_h
#define ScnGpuDevice_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/core/ScnMemElastic.h"
#include "ixrender/gpu/ScnGpuBuffer.h"
#include "ixrender/gpu/ScnGpuVertexbuff.h"
#include "ixrender/gpu/ScnGpuFramebuff.h"
#include "ixrender/gpu/ScnGpuRenderJob.h"
#include "ixrender/gpu/ScnGpuDeviceDesc.h"

#ifdef __cplusplus
extern "C" {
#endif

//ENScnGpuDevicePower

typedef enum ENScnGpuDevicePower {
    ENScnGpuDevicePower_Unknown = 0,
    ENScnGpuDevicePower_Low,
    ENScnGpuDevicePower_High,
    //Count
    ENScnGpuDevicePower_Count
} ENScnGpuDevicePower;

//ENScnGpuDeviceRemovable

typedef enum ENScnGpuDeviceRemovable {
    ENScnGpuDeviceRemovable_Unknown = 0,
    ENScnGpuDeviceRemovable_No,
    ENScnGpuDeviceRemovable_Yes,
    //Count
    ENScnGpuDeviceRemovable_Count
} ENScnGpuDeviceRemovable;

//ENScnGpuDeviceHeadless

typedef enum ENScnGpuDeviceHeadless {
    ENScnGpuDeviceHeadless_Unknown = 0,
    ENScnGpuDeviceHeadless_No,
    ENScnGpuDeviceHeadless_Yes,
    //Count
    ENScnGpuDeviceHeadless_Count
} ENScnGpuDeviceHeadless;

//ENScnGpuDeviceUnifiedMem

typedef enum ENScnGpuDeviceUnifiedMem {
    ENScnGpuDeviceUnifiedMem_Unknown = 0,
    ENScnGpuDeviceUnifiedMem_No,
    ENScnGpuDeviceUnifiedMem_Yes,
    //Count
    ENScnGpuDeviceUnifiedMem_Count
} ENScnGpuDeviceUnifiedMem;

//ENScnGpuDeviceCompute

typedef enum ENScnGpuDeviceCompute {
    ENScnGpuDeviceCompute_Unknown = 0,
    ENScnGpuDeviceCompute_No,
    ENScnGpuDeviceCompute_Yes,
    //Count
    ENScnGpuDeviceCompute_Count
} ENScnGpuDeviceCompute;

//STScnGpuDeviceCfg

#define STScnGpuDeviceCfg_Zero   { ENScnGpuDevicePower_Unknown, ENScnGpuDeviceRemovable_Unknown, ENScnGpuDeviceHeadless_Unknown, ENScnGpuDeviceUnifiedMem_Unknown, ENScnGpuDeviceCompute_Unknown }

typedef struct STScnGpuDeviceCfg {
    //preferences
    ENScnGpuDevicePower         power;
    ENScnGpuDeviceRemovable     removable;
    ENScnGpuDeviceHeadless      headless;
    ENScnGpuDeviceUnifiedMem    unifiedMem;
    ENScnGpuDeviceCompute       compute;
} STScnGpuDeviceCfg;

//ScnGpuDeviceRef

//ScnGpuBufferRef

/** @struct ScnGpuDeviceRef
 *  @brief ScnGpuDevice shared pointer. An abstract object based on the currently used API.
 */

#define ScnGpuDeviceRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnGpuDevice)

//STScnGpuDeviceApiItf

/** @struct STScnGpuDeviceApiItf
 *  @brief Device's API interface.
 *  @var STScnGpuDeviceApiItf::free
 *  Method to free the device.
 *  @var STScnGpuDeviceApiItf::getApiDevice
 *  Method to retrieve the device's abstract object.
 *  @var STScnGpuDeviceApiItf::getDesc
 *  Method to retrieve the device's description.
 *  @var STScnGpuDeviceApiItf::allocBuffer
 *  Method to allocate a buffer.
 *  @var STScnGpuDeviceApiItf::allocVertexBuff
 *  Method to allocate a vertex buffer.
 *  @var STScnGpuDeviceApiItf::allocFramebuffFromOSView
 *  Method to allocate a frame buffer.
 *  @var STScnGpuDeviceApiItf::allocTexture
 *  Method to allocate a texture.
 *  @var STScnGpuDeviceApiItf::allocSampler
 *  Method to allocate a sampler.
 *  @var STScnGpuDeviceApiItf::allocRenderJob
 *  Method to allocate a render job..
 */

typedef struct STScnGpuDeviceApiItf {
    void                (*free)(void* obj);
    void*               (*getApiDevice)(void* obj);
    STScnGpuDeviceDesc  (*getDesc)(void* obj);
    ScnGpuBufferRef     (*allocBuffer)(void* obj, ScnMemElasticRef mem);
    ScnGpuVertexbuffRef (*allocVertexBuff)(void* obj, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff);
    ScnGpuFramebuffRef  (*allocFramebuffFromOSView)(void* obj, void* mtkView);
    ScnGpuTextureRef    (*allocTexture)(void* obj, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData);
    ScnGpuSamplerRef    (*allocSampler)(void* obj, const STScnGpuSamplerCfg* const cfg);
    ScnGpuRenderJobRef  (*allocRenderJob)(void* obj);
} STScnGpuDeviceApiItf;

//

/**
 * @brief Prepares the gpu abstract object with the provided interface.
 * @param ref Reference to object.
 * @param itf Interface to the API.
 * @param itfParam Parameter to be given to the interface's  methods.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnGpuDevice_prepare(ScnGpuDeviceRef ref, const STScnGpuDeviceApiItf* itf, void* itfParam);

/**
 * @brief Retrieves the gpu abstract object pointer.
 * @param ref Reference to object.
 * @return Abstract object's pointer on success, NULL otherwise.
 */
void* ScnGpuDevice_getApiDevice(ScnGpuDeviceRef ref);

/**
 * @brief Retrieves the gpu device's description.
 * @param ref Reference to object.
 * @return Gpu description on success, STScnGpuDeviceDesc_Zero otherwise.
 */
STScnGpuDeviceDesc ScnGpuDevice_getDesc(ScnGpuDeviceRef ref);

/**
 * @brief Allocates a new gpu buffer attached to this device.
 * @param ref Reference to object.
 * @param optMem Memory object with the initial content for the buffer, optional.
 * @return A gpu buffer on success, ScnGpuBufferRef_Zero otherwise.
 */
ScnGpuBufferRef ScnGpuDevice_allocBuffer(ScnGpuDeviceRef ref, ScnMemElasticRef optMem);

/**
 * @brief Allocates a new gpu vextex buffer attached to this device.
 * @param ref Reference to object.
 * @param cfg Vertex buffer configuration.
 * @param vBuff Gpu buffer for vertices storage.
 * @param idxBuff Gpu buffer for indices storage.
 * @return A gpu vertex buffer on success, ScnGpuVertexbuffRef_Zero otherwise.
 */
ScnGpuVertexbuffRef ScnGpuDevice_allocVertexBuff(ScnGpuDeviceRef ref, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff);

/**
 * @brief Allocates a new gpu frame buffer attached to this device.
 * @param ref Reference to object.
 * @param mtkView Operating system abstract view object (MTKView for Metal, ...)
 * @return A gpu frame buffer on success, ScnGpuFramebuffRef_Zero otherwise.
 */
ScnGpuFramebuffRef ScnGpuDevice_allocFramebuffFromOSView(ScnGpuDeviceRef ref, void* mtkView);

/**
 * @brief Allocates a new gpu texture attached to this device.
 * @param ref Reference to object.
 * @param cfg Texture configuration.
 * @param optSrcProps Texture's initial data properties, optional.
 * @param optSrcData Texture's initial data payload, optional.
 * @return A gpu texture on success, ScnGpuTextureRef_Zero otherwise.
 */
ScnGpuTextureRef ScnGpuDevice_allocTexture(ScnGpuDeviceRef ref, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const optSrcProps, const void* optSrcData);

/**
 * @brief Allocates a new gpu sampler attached to this device.
 * @param ref Reference to object.
 * @param cfg Sampler configuration.
 * @return A gpu sampler on success, ScnGpuSamplerRef_Zero otherwise.
 */
ScnGpuSamplerRef ScnGpuDevice_allocSampler(ScnGpuDeviceRef ref, const STScnGpuSamplerCfg* const cfg);

/**
 * @brief Allocates a new gpu render job attached to this device.
 * @param ref Reference to object.
 * @return A gpu render job on success, ScnGpuRenderJobRef_Zero otherwise.
 */
ScnGpuRenderJobRef ScnGpuDevice_allocRenderJob(ScnGpuDeviceRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuDevice_h */
