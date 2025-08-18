//
//  ScnApiMetalDevice.h
//  ixtli-render
//
//  Created by Marcos Ortega on 18/8/25.
//

#ifndef ScnApiMetalDevice_h
#define ScnApiMetalDevice_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnContext.h"
#include "ixrender/api/ScnApiItf.h"
//
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import <TargetConditionals.h>  //for TARGET_OS_* macros

#ifdef __cplusplus
extern "C" {
#endif

//STScnApiMetalDevice

typedef struct STScnApiMetalDevice {
    ScnContextRef   ctx;
    STScnApiItf     itf;
    id<MTLDevice>   dev;
    id<MTLLibrary>  lib;
    id<MTLCommandQueue> cmdQueue;
} STScnApiMetalDevice;

ScnGpuDeviceRef     ScnApiMetal_allocDevice(ScnContextRef ctx, const STScnGpuDeviceCfg* cfg);
void                ScnApiMetalDevice_free(void* obj);
void*               ScnApiMetalDevice_getApiDevice(void* obj);
STScnGpuDeviceDesc  ScnApiMetalDevice_getDesc(void* obj);
ScnGpuBufferRef     ScnApiMetalDevice_allocBuffer(void* obj, ScnMemElasticRef mem);
ScnGpuVertexbuffRef ScnApiMetalDevice_allocVertexBuff(void* obj, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff);
ScnGpuFramebuffRef  ScnApiMetalDevice_allocFramebuffFromOSView(void* obj, void* mtkView);
ScnGpuTextureRef    ScnApiMetalDevice_allocTexture(void* obj, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData);
ScnGpuSamplerRef    ScnApiMetalDevice_allocSampler(void* obj, const STScnGpuSamplerCfg* const cfg);
ScnGpuRenderJobRef  ScnApiMetalDevice_allocRenderJob(void* obj);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiMetalDevice_h */
