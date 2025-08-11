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

#define ScnGpuDeviceRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnGpuDevice)

//STScnGpuDeviceApiItf

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

ScnBOOL             ScnGpuDevice_prepare(ScnGpuDeviceRef ref, const STScnGpuDeviceApiItf* itf, void* itfParam);
void*               ScnGpuDevice_getApiDevice(ScnGpuDeviceRef ref);
STScnGpuDeviceDesc  ScnGpuDevice_getDesc(ScnGpuDeviceRef ref);
ScnGpuBufferRef     ScnGpuDevice_allocBuffer(ScnGpuDeviceRef ref, ScnMemElasticRef mem);
ScnGpuVertexbuffRef ScnGpuDevice_allocVertexBuff(ScnGpuDeviceRef ref, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff);
ScnGpuFramebuffRef  ScnGpuDevice_allocFramebuffFromOSView(ScnGpuDeviceRef ref, void* mtkView);
ScnGpuTextureRef    ScnGpuDevice_allocTexture(ScnGpuDeviceRef ref, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData);
ScnGpuSamplerRef    ScnGpuDevice_allocSampler(ScnGpuDeviceRef ref, const STScnGpuSamplerCfg* const cfg);
ScnGpuRenderJobRef  ScnGpuDevice_allocRenderJob(ScnGpuDeviceRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuDevice_h */
