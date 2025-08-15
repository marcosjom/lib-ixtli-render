//
//  ScnApiMetal.m
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/api/ScnApiMetal.h"
#include "ixrender/gpu/ScnGpuDevice.h"
#include "ixrender/gpu/ScnGpuBuffer.h"
#include "ixrender/gpu/ScnGpuSampler.h"
#include "ixrender/gpu/ScnGpuTexture.h"
#include "ixrender/scene/ScnRenderCmd.h"
//
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import <TargetConditionals.h>  //for TARGET_OS_* macros

#ifdef SCN_ASSERTS_ACTIVATED
#include <string.h> //strncmp()
#endif


//STScnGpuDeviceApiItf

ScnGpuDeviceRef     ScnApiMetal_allocDevice(ScnContextRef ctx, const STScnGpuDeviceCfg* cfg);
void                ScnApiMetal_device_free(void* obj);
void*               ScnApiMetal_device_getApiDevice(void* obj);
STScnGpuDeviceDesc  ScnApiMetal_device_getDesc(void* obj);
ScnGpuBufferRef     ScnApiMetal_device_allocBuffer(void* obj, ScnMemElasticRef mem);
ScnGpuVertexbuffRef ScnApiMetal_device_allocVertexBuff(void* obj, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff);
ScnGpuFramebuffRef  ScnApiMetal_device_allocFramebuffFromOSView(void* obj, void* mtkView);
ScnGpuTextureRef    ScnApiMetal_device_allocTexture(void* obj, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData);
ScnGpuSamplerRef    ScnApiMetal_device_allocSampler(void* obj, const STScnGpuSamplerCfg* const cfg);
ScnGpuRenderJobRef  ScnApiMetal_device_allocRenderJob(void* obj);
//buffer
void                ScnApiMetal_buffer_free(void* data);
ScnBOOL             ScnApiMetal_buffer_sync(void* data, ScnMemElasticRef mem, const STScnGpuBufferChanges* changes);
//vertexbuff
void                ScnApiMetal_vertexbuff_free(void* data);
ScnBOOL             ScnApiMetal_vertexbuff_sync(void* data, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff);
ScnBOOL             ScnApiMetal_vertexbuff_activate(void* data);
ScnBOOL             ScnApiMetal_vertexbuff_deactivate(void* data);
//frameBuffer (view)
void                ScnApiMetal_framebuff_view_free(void* data);
STScnSize2DU        ScnApiMetal_framebuff_view_getSize(void* pObj);
ScnBOOL             ScnApiMetal_framebuff_view_syncSize(void* pObj, const STScnSize2DU size);
STScnGpuFramebuffProps ScnApiMetal_framebuff_view_getProps(void* data);
ScnBOOL             ScnApiMetal_framebuff_view_setProps(void* data, const STScnGpuFramebuffProps* const props);
//sampler
void                ScnApiMetal_sampler_free(void* pObj);
STScnGpuSamplerCfg  ScnApiMetal_sampler_getCfg(void* data);
//texture
void                ScnApiMetal_texture_free(void* pObj);
ScnBOOL             ScnApiMetal_texture_sync(void* data, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData, const STScnGpuTextureChanges* const changes);
//render job
void                ScnApiMetal_renderJob_free(void* data);
ENScnGpuRenderJobState ScnApiMetal_renderJob_getState(void* data);
ScnBOOL             ScnApiMetal_renderJob_buildBegin(void* data, ScnGpuBufferRef bPropsScns, ScnGpuBufferRef bPropsMdls);
ScnBOOL             ScnApiMetal_renderJob_buildAddCmds(void* data, const struct STScnRenderCmd* const cmds, const ScnUI32 cmdsSz);
ScnBOOL             ScnApiMetal_renderJob_buildEndAndEnqueue(void* data);

ScnBOOL ScnApiMetal_getApiItf(STScnApiItf* dst){
    if(dst == NULL) return ScnFALSE;
    //
    ScnMemory_setZeroSt(*dst);
    //gobal
    dst->allocDevice        = ScnApiMetal_allocDevice;
    //device
    dst->dev.free           = ScnApiMetal_device_free;
    dst->dev.getApiDevice   = ScnApiMetal_device_getApiDevice;
    dst->dev.getDesc        = ScnApiMetal_device_getDesc;
    dst->dev.allocBuffer    = ScnApiMetal_device_allocBuffer;
    dst->dev.allocVertexBuff = ScnApiMetal_device_allocVertexBuff;
    dst->dev.allocFramebuffFromOSView = ScnApiMetal_device_allocFramebuffFromOSView;
    dst->dev.allocTexture   = ScnApiMetal_device_allocTexture;
    dst->dev.allocSampler   = ScnApiMetal_device_allocSampler;
    dst->dev.allocRenderJob = ScnApiMetal_device_allocRenderJob;
    //buffer
    dst->buff.free          = ScnApiMetal_buffer_free;
    dst->buff.sync          = ScnApiMetal_buffer_sync;
    //vertexbuff
    dst->vertexBuff.free    = ScnApiMetal_vertexbuff_free;
    dst->vertexBuff.sync    = ScnApiMetal_vertexbuff_sync;
    dst->vertexBuff.activate = ScnApiMetal_vertexbuff_activate;
    dst->vertexBuff.deactivate = ScnApiMetal_vertexbuff_deactivate;
    //
    return ScnTRUE;
}

// STScnApiMetalSampler

typedef struct STScnApiMetalSampler {
    ScnContextRef           ctx;
    STScnGpuSamplerCfg      cfg;
    id<MTLSamplerState>     smplr;
} STScnApiMetalSampler;

//STScnApiMetalDevice

typedef struct STScnApiMetalDevice {
    ScnContextRef   ctx;
    STScnApiItf     itf;
    id<MTLDevice>   dev;
    id<MTLLibrary>  lib;
    id<MTLCommandQueue> cmdQueue;
} STScnApiMetalDevice;

ScnGpuDeviceRef ScnApiMetal_allocDevice(ScnContextRef ctx, const STScnGpuDeviceCfg* cfg){
    ScnGpuDeviceRef r = ScnObjRef_Zero;
    if(!ScnContext_isNull(ctx)){
        id<MTLDevice> dev = nil;
        id<MTLCommandQueue> cmdQueue = nil;
        //Search device
#       if TARGET_OS_OSX
        {
            NSArray<id<MTLDevice>> *devs = MTLCopyAllDevices();
            if(devs == nil){
                SCN_PRINTF_ERROR("Metal, could not retrieve devices.\n");
            } else {
                ScnBOOL devIsExplicitMatch = ScnFALSE;
                SCN_PRINTF_INFO("Metal, %d devices found:\n", (int)devs.count);
                ScnUI32 i; for(i = 0; i < devs.count; i++){
                    id<MTLDevice> d = devs[i];
                    SCN_PRINTF_INFO("    ---------\n");
                    SCN_PRINTF_INFO("    Dev#%d/%d: '%s'\n", (i + 1), (int)devs.count, [d.name UTF8String]);
                    //Identification
                    if(@available(iOS 18, macOS 14.0, *)){
                        SCN_PRINTF_INFO("         Arch: '%s'\n", [d.architecture.name UTF8String]);
                    }
                    SCN_PRINTF_INFO("          Loc: '%s' (num: %d)\n", d.location == MTLDeviceLocationBuiltIn ? "BuiltIn" : d.location == MTLDeviceLocationSlot ? "Slot" : d.location == MTLDeviceLocationExternal ? "External" : d.location == MTLDeviceLocationUnspecified ? "Unspecified" :"Unknown", (int)d.locationNumber);
                    SCN_PRINTF_INFO("       LowPwr: %s\n", d.isLowPower ? "yes" : "no");
                    SCN_PRINTF_INFO("    Removable: %s\n", d.isRemovable ? "yes" : "no");
                    SCN_PRINTF_INFO("     Headless: %s\n", d.isHeadless ? "yes" : "no");
                    SCN_PRINTF_INFO("         Peer: grpId(%llu) idx(%d) count(%d)\n", (ScnUI64)d.peerGroupID, d.peerIndex, d.peerCount);
                    //GPU's Device Memory
                    SCN_PRINTF_INFO("     CurAlloc: %.2f %s\n", (double)d.currentAllocatedSize / (d.currentAllocatedSize >= (1024 * 1024 * 1024) ? (double)(1024 * 1024 * 1024) : d.currentAllocatedSize >= (1024 * 1024) ? (double)(1024 * 1024) : d.currentAllocatedSize >= (1024) ? (double)(1024) : 1.0), (d.currentAllocatedSize >= (1024 * 1024 * 1024) ? "GBs" : d.currentAllocatedSize >= (1024 * 1024) ? "MBs" : d.currentAllocatedSize >= (1024) ? "KBs" : "bytes"));
                    SCN_PRINTF_INFO("     MaxAlloc: %.2f %s (recommended)\n", (double)d.recommendedMaxWorkingSetSize / (d.recommendedMaxWorkingSetSize >= (1024 * 1024 * 1024) ? (double)(1024 * 1024 * 1024) : d.recommendedMaxWorkingSetSize >= (1024 * 1024) ? (double)(1024 * 1024) : d.recommendedMaxWorkingSetSize >= (1024) ? (double)(1024) : 1.0), (d.recommendedMaxWorkingSetSize >= (1024 * 1024 * 1024) ? "GBs" : d.recommendedMaxWorkingSetSize >= (1024 * 1024) ? "MBs" : d.recommendedMaxWorkingSetSize >= (1024) ? "KBs" : "bytes"));
                    SCN_PRINTF_INFO("      MaxRate: %.2f %s/s\n", (double)d.maxTransferRate / (d.maxTransferRate >= (1024 * 1024 * 1024) ? (double)(1024 * 1024 * 1024) : d.maxTransferRate >= (1024 * 1024) ? (double)(1024 * 1024) : d.maxTransferRate >= (1024) ? (double)(1024) : 1.0), (d.maxTransferRate >= (1024 * 1024 * 1024) ? "GBs" : d.maxTransferRate >= (1024 * 1024) ? "MBs" : d.maxTransferRate >= (1024) ? "KBs" : "bytes"));
                    SCN_PRINTF_INFO("   UnifiedMem: %s\n", d.hasUnifiedMemory ? "yes" : "no");
                    //Compute Support
                    SCN_PRINTF_INFO(" ThreadGrpMem: %.2f %s\n", (double)d.maxThreadgroupMemoryLength / (d.maxThreadgroupMemoryLength >= (1024 * 1024 * 1024) ? (double)(1024 * 1024 * 1024) : d.maxThreadgroupMemoryLength >= (1024 * 1024) ? (double)(1024 * 1024) : d.maxThreadgroupMemoryLength >= (1024) ? (double)(1024) : 1.0), (d.maxThreadgroupMemoryLength >= (1024 * 1024 * 1024) ? "GBs" : d.maxThreadgroupMemoryLength >= (1024 * 1024) ? "MBs" : d.maxThreadgroupMemoryLength >= (1024) ? "KBs" : "bytes"));
                    SCN_PRINTF_INFO("  ThrdsPerGrp: (%d, %d, %d)\n", (int)d.maxThreadsPerThreadgroup.width, (int)d.maxThreadsPerThreadgroup.height, (int)d.maxThreadsPerThreadgroup.depth);
                    //Functions Pointer Support
                    SCN_PRINTF_INFO("     FuncPtrs: %s (compute kernel functions)\n", d.supportsFunctionPointers ? "yes" : "no");
                    if(@available(iOS 18, macOS 12.0, *)){
                        SCN_PRINTF_INFO("    FPtrsRndr: %s\n", d.supportsFunctionPointersFromRender ? "yes" : "no");
                    }
                    //Texture and sampler support
                    SCN_PRINTF_INFO("   32bFltFilt: %s\n", d.supports32BitFloatFiltering ? "yes" : "no");
                    SCN_PRINTF_INFO("   BCTextComp: %s\n", d.supportsBCTextureCompression ? "yes" : "no");
                    SCN_PRINTF_INFO("    Depth24-8: %s\n", d.isDepth24Stencil8PixelFormatSupported ? "yes" : "no");
                    SCN_PRINTF_INFO("  TexLODQuery: %s\n", d.supportsQueryTextureLOD ? "yes" : "no");
                    SCN_PRINTF_INFO("        RWTex: %s\n", d.readWriteTextureSupport ? "yes" : "no");
                    //Render support
                    SCN_PRINTF_INFO("   RayTracing: %s\n", d.supportsRaytracing ? "yes" : "no");
                    if(@available(iOS 18, macOS 12.0, *)){
                        SCN_PRINTF_INFO("  RayTracRndr: %s\n", d.supportsRaytracingFromRender ? "yes" : "no");
                    }
                    SCN_PRINTF_INFO("  PrimMotBlur: %s\n", d.supportsPrimitiveMotionBlur ? "yes" : "no");
                    SCN_PRINTF_INFO("      32bMSAA: %s\n", d.supports32BitMSAA ? "yes" : "no");
                    SCN_PRINTF_INFO("  PullModeInt: %s\n", d.supportsPullModelInterpolation ? "yes" : "no");
                    SCN_PRINTF_INFO("ShadBaryCoord: %s\n", d.supportsShaderBarycentricCoordinates ? "yes" : "no");
                    SCN_PRINTF_INFO("  ProgSmplPos: %s\n", d.areProgrammableSamplePositionsSupported ? "yes" : "no");
                    SCN_PRINTF_INFO("  RstrOrdGrps: %s\n", d.areRasterOrderGroupsSupported ? "yes" : "no");
                    //
                    const ScnSI32 cfgExplictOptsCount =
                    (
                     (cfg != NULL && cfg->headless > ENScnGpuDeviceHeadless_Unknown && cfg->headless < ENScnGpuDeviceHeadless_Count ? 1 : 0)
                     + (cfg != NULL && cfg->removable > ENScnGpuDeviceRemovable_Unknown && cfg->removable < ENScnGpuDeviceRemovable_Count ? 1 : 0)
                     + (cfg != NULL && cfg->unifiedMem > ENScnGpuDeviceUnifiedMem_Unknown && cfg->unifiedMem < ENScnGpuDeviceUnifiedMem_Count ? 1 : 0)
                     + (cfg != NULL && cfg->compute > ENScnGpuDeviceCompute_Unknown && cfg->compute < ENScnGpuDeviceCompute_Count ? 1 : 0)
                     );
                    const ScnBOOL isCfgExplicitMatch =
                    (
                     cfgExplictOptsCount > 0
                     && (cfg->headless == ENScnGpuDeviceHeadless_Unknown || (cfg->headless == ENScnGpuDeviceHeadless_Yes && d.isHeadless) || (cfg->headless == ENScnGpuDeviceHeadless_No && !d.isHeadless))
                     && (cfg->removable == ENScnGpuDeviceRemovable_Unknown || (cfg->removable == ENScnGpuDeviceRemovable_Yes && d.isRemovable) || (cfg->removable == ENScnGpuDeviceRemovable_No && !d.isRemovable))
                     && (cfg->unifiedMem == ENScnGpuDeviceUnifiedMem_Unknown || (cfg->unifiedMem == ENScnGpuDeviceUnifiedMem_Yes && d.hasUnifiedMemory) || (cfg->unifiedMem == ENScnGpuDeviceUnifiedMem_No && !d.hasUnifiedMemory))
                     && (cfg->compute == ENScnGpuDeviceCompute_Unknown || (cfg->compute == ENScnGpuDeviceCompute_Yes && (d.maxThreadsPerThreadgroup.width * d.maxThreadsPerThreadgroup.height) > 0) || (cfg->compute == ENScnGpuDeviceCompute_No && (d.maxThreadsPerThreadgroup.width * d.maxThreadsPerThreadgroup.height) == 0))
                     );
                    if(
                       dev == nil //first device is default
                       || (!devIsExplicitMatch && isCfgExplicitMatch) //force explicit match only
                       || (dev.isHeadless && !d.isHeadless) //allways prefer non-headless devices
                       || (dev.isRemovable && !d.isRemovable) //allways prefer non-removable devices
                       || (!dev.hasUnifiedMemory && d.hasUnifiedMemory) //allways prefer unified memory devices
                       )
                    {
                        [dev retain];
                        if(dev != nil){ [dev release]; }
                        dev = d;
                        devIsExplicitMatch = isCfgExplicitMatch;
                    }
                }
                if(devs.count > 0){
                    SCN_PRINTF_INFO("    ---------\n");
                }
            }
            if(devs != nil){
                [devs release];
                devs = nil;
            }
        }
#       endif
        if(dev == nil){
            //Use default device
            dev = MTLCreateSystemDefaultDevice();
        }
        if(dev == nil){
            SCN_PRINTF_ERROR("Metal, could select a device.\n");
        } else {
            SCN_PRINTF_INFO("Selected device: '%s'\n", [dev.name UTF8String]);
            id<MTLLibrary> defLib = [dev newDefaultLibrary];
            if (defLib == nil){
                SCN_PRINTF_ERROR("Metal, newDefaultLibrary failed.\n");
            } else if(nil == (cmdQueue = [dev newCommandQueue])){
                SCN_PRINTF_ERROR("Metal, newCommandQueue failed.\n");
            } else {
                STScnApiMetalDevice* obj = (STScnApiMetalDevice*)ScnContext_malloc(ctx, sizeof(STScnApiMetalDevice), SCN_DBG_STR("STScnApiMetalDevice"));
                if(obj == NULL){
                    SCN_PRINTF_ERROR("ScnContext_malloc(STScnApiMetalDevice) failed.\n");
                } else {
                    //init STScnApiMetalDevice
                    ScnMemory_setZeroSt(*obj);
                    ScnContext_set(&obj->ctx, ctx);
                    obj->dev    = dev; [dev retain];
                    obj->lib    = defLib; [defLib retain];
                    obj->cmdQueue = cmdQueue; [cmdQueue retain];
                    //
                    if(!ScnApiMetal_getApiItf(&obj->itf)){
                        SCN_PRINTF_ERROR("ScnApiMetal_allocDevice::ScnApiMetal_getApiItf failed.\n");
                    } else {
                        ScnGpuDeviceRef d = ScnGpuDevice_alloc(ctx);
                        if(!ScnGpuDevice_isNull(d)){
                            if(!ScnGpuDevice_prepare(d, &obj->itf.dev, obj)){
                                SCN_PRINTF_ERROR("ScnApiMetal_allocDevice::ScnGpuDevice_prepare failed.\n");
                            } else {
                                ScnGpuDevice_set(&r, d);
                                obj = NULL; //consume
                            }
                            ScnGpuDevice_releaseAndNull(&d);
                        }
                    }
                }
                //release (if not consumed)
                if(obj != NULL){
                    ScnApiMetal_device_free(obj);
                    obj = NULL;
                }
            }
            //release (if not consumed)
            if(defLib != nil){
                [defLib release];
                defLib = nil;
            }
        }
        //release (if not consumed)
        if(cmdQueue != nil){
            [cmdQueue release];
            cmdQueue = nil;
        }
        if(dev != nil){
            [dev release];
            dev = nil;
        }
    }
    return r;
}

void ScnApiMetal_device_free(void* pObj){
    STScnApiMetalDevice* obj = (STScnApiMetalDevice*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        if(obj->cmdQueue != nil){
            [obj->cmdQueue release];
            obj->cmdQueue = nil;
        }
        if(obj->lib != nil){
            [obj->lib release];
            obj->lib = nil;
        }
        if(obj->dev != nil){
            [obj->dev release];
            obj->dev = nil;
        }
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

// STScnApiMetalSampler

ScnGpuSamplerRef ScnApiMetal_device_allocSampler(void* pObj, const STScnGpuSamplerCfg* const cfg){
    ScnGpuSamplerRef r = ScnGpuSamplerRef_Zero;
    STScnApiMetalDevice* dev = (STScnApiMetalDevice*)pObj;
    if(dev != NULL && dev->dev != nil && cfg != NULL){
        MTLSamplerDescriptor* desc = [MTLSamplerDescriptor new];
        if(desc == nil){
            SCN_PRINTF_ERROR("[MTLSamplerDescriptor new] failed.\n");
        } else {
            const MTLSamplerAddressMode addressMode = (cfg->address == ENScnGpusamplerAddress_Clamp ? MTLSamplerAddressModeClampToEdge : MTLSamplerAddressModeRepeat);
            desc.minFilter = (cfg->magFilter == ENScnGpuSamplerFilter_Linear ? MTLSamplerMinMagFilterLinear : MTLSamplerMinMagFilterNearest);
            desc.magFilter = (cfg->minFilter == ENScnGpuSamplerFilter_Linear ? MTLSamplerMinMagFilterLinear : MTLSamplerMinMagFilterNearest);
            desc.sAddressMode = desc.tAddressMode = desc.rAddressMode = addressMode;
            id<MTLSamplerState> sampler = [dev->dev newSamplerStateWithDescriptor:desc];
            if(sampler == nil){
                SCN_PRINTF_ERROR("[dev newSamplerStateWithDescriptor] failed.\n");
            } else {
                STScnApiMetalSampler* obj = (STScnApiMetalSampler*)ScnContext_malloc(dev->ctx, sizeof(STScnApiMetalSampler), SCN_DBG_STR("STScnApiMetalSampler"));
                if(obj == NULL){
                    SCN_PRINTF_ERROR("ScnContext_malloc(STScnApiMetalSampler) failed.\n");
                } else {
                    ScnMemory_setZeroSt(*obj);
                    ScnContext_set(&obj->ctx, dev->ctx);
                    obj->cfg    = *cfg;
                    obj->smplr  = sampler; [sampler retain];
                    //
                    ScnGpuSamplerRef s = ScnGpuSampler_alloc(dev->ctx);
                    if(!ScnGpuSampler_isNull(s)){
                        STScnGpuSamplerApiItf itf;
                        ScnMemory_setZeroSt(itf);
                        itf.free    = ScnApiMetal_sampler_free;
                        itf.getCfg  = ScnApiMetal_sampler_getCfg;
                        if(!ScnGpuSampler_prepare(s, &itf, obj)){
                            SCN_PRINTF_ERROR("ScnApiMetal_device_allocSampler::ScnGpuSampler_prepare failed.\n");
                        } else {
                            ScnGpuSampler_set(&r, s);
                            obj = NULL; //consume
                        }
                        ScnGpuSampler_releaseAndNull(&s);
                    }
                }
                //release (if not consumed)
                if(obj != NULL){
                    ScnApiMetal_sampler_free(obj);
                    obj = NULL;
                }
                //
                [sampler release];
                sampler = nil;
            }
            //
            [desc release];
            desc = nil;
        }
    }
    return r;
}

void ScnApiMetal_sampler_free(void* pObj){
    STScnApiMetalSampler* obj = (STScnApiMetalSampler*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        if(obj->smplr != nil){
            [obj->smplr release];
            obj->smplr = nil;
        }
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

STScnGpuSamplerCfg ScnApiMetal_sampler_getCfg(void* pObj){
    STScnGpuSamplerCfg r = STScnGpuSamplerCfg_Zero;
    STScnApiMetalSampler* obj = (STScnApiMetalSampler*)pObj;
    if(obj != NULL ){
        r = obj->cfg;
    }
    return r;
}

//STScnApiMetalBuffer

typedef struct STScnApiMetalBuffer {
    ScnContextRef   ctx;
    STScnApiItf     itf;
    id<MTLDevice>   dev;
    id<MTLBuffer>   buff;
} STScnApiMetalBuffer;

void* ScnApiMetal_device_getApiDevice(void* pObj){
    STScnApiMetalDevice* dev = (STScnApiMetalDevice*)pObj;
    return (dev != NULL ? dev->dev : NULL);
}

STScnGpuDeviceDesc ScnApiMetal_device_getDesc(void* obj){
    STScnGpuDeviceDesc r = STScnGpuDeviceDesc_Zero;
    {
        r.isTexFmtInfoRequired  = ScnTRUE;  //fragment shader requires the textures format info to produce correct color output
        r.offsetsAlign          = 32;       //buffers offsets aligment
        r.memBlockAlign         = 256;      //buffer memory copy alignment
    }
    return r;
}

ScnBOOL ScnApiMetal_buffer_syncRanges_(id<MTLBuffer> buff, ScnMemElasticRef mem, const STScnRangeU* const rngs, const ScnUI32 rngsUse);

ScnGpuBufferRef ScnApiMetal_device_allocBuffer(void* pObj, ScnMemElasticRef mem){
    ScnGpuBufferRef r = ScnObjRef_Zero;
    STScnApiMetalDevice* dev = (STScnApiMetalDevice*)pObj;
    if(dev != NULL && dev->dev != NULL && !ScnMemElastic_isNull(mem)){
        const ScnUI32 cpuBuffSz = ScnMemElastic_getAddressableSize(mem);
        if(cpuBuffSz <= 0){
            SCN_PRINTF_ERROR("allocating zero-sz gpu buffer is not allowed.\n");
        } else {
            id<MTLBuffer> buff = [dev->dev newBufferWithLength:cpuBuffSz options:MTLResourceStorageModeShared | MTLResourceCPUCacheModeWriteCombined];
            if(buff != nil){
                const STScnRangeU rngAll = ScnMemElastic_getUsedAddressesRngAligned(mem);
                if(!ScnApiMetal_buffer_syncRanges_(buff, mem, &rngAll, 1)){
                    SCN_PRINTF_ERROR("ScnApiMetal_buffer_syncRanges_ failed.\n");
                } else {
                    //synced
                    STScnApiMetalBuffer* obj = (STScnApiMetalBuffer*)ScnContext_malloc(dev->ctx, sizeof(STScnApiMetalBuffer), SCN_DBG_STR("STScnApiMetalBuffer"));
                    if(obj == NULL){
                        SCN_PRINTF_ERROR("ScnContext_malloc(STScnApiMetalBuffer) failed.\n");
                    } else {
                        ScnMemory_setZeroSt(*obj);
                        ScnContext_set(&obj->ctx, dev->ctx);
                        obj->itf        = dev->itf;
                        obj->dev        = dev->dev;  //retain?
                        obj->buff       = buff; [buff retain];
                        //
                        ScnGpuBufferRef d = ScnGpuBuffer_alloc(dev->ctx);
                        if(!ScnGpuBuffer_isNull(d)){
                            if(!ScnGpuBuffer_prepare(d, &obj->itf.buff, obj)){
                                SCN_PRINTF_ERROR("ScnApiMetal_allocDevice::ScnGpuBuffer_prepare failed.\n");
                            } else {
                                ScnGpuBuffer_set(&r, d);
                                obj = NULL; //consume
                            }
                            ScnGpuBuffer_releaseAndNull(&d);
                        }
                    }
                    //release (if not consumed)
                    if(obj != NULL){
                        ScnApiMetal_buffer_free(obj);
                        obj = NULL;
                    }
                }
                [buff release];
                buff = nil;
            }
        }
    }
    return r;
}

void ScnApiMetal_buffer_free(void* pObj){
    STScnApiMetalBuffer* obj = (STScnApiMetalBuffer*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        if(obj->buff != nil){
            [obj->buff release];
            obj->buff = nil;
        }
        if(obj->dev != nil){
            //release?
            obj->dev = nil;
        }
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

#ifdef SCN_ASSERTS_ACTIVATED
ScnBOOL ScnApiMetal_buffer_syncValidate_(STScnApiMetalBuffer* obj, ScnMemElasticRef mem);
#endif

ScnBOOL ScnApiMetal_buffer_sync(void* pObj, ScnMemElasticRef mem, const STScnGpuBufferChanges* changes){
    ScnBOOL r = ScnFALSE;
    STScnApiMetalBuffer* obj = (STScnApiMetalBuffer*)pObj;
    if(obj->buff == nil || ScnMemElastic_isNull(mem)){
        //missing params
        return ScnFALSE;
    }
    //sync
    {
        ScnBOOL buffIsNew = ScnFALSE;
        ScnUI32 buffLen = (ScnUI32)obj->buff.length;
        const ScnUI32 cpuBuffSz = ScnMemElastic_getAddressableSize(mem);
        //resize (if necesary)
        if(cpuBuffSz != buffLen){
            //recreate buffer
            id<MTLBuffer> buff = [obj->dev newBufferWithLength:cpuBuffSz options:MTLResourceStorageModeShared | MTLResourceCPUCacheModeWriteCombined];
            if(buff != nil){
                SCN_PRINTF_VERB("ScnApiMetal_buffer_sync::gpu-buff resized from %u to %u bytes.\n", buffLen, cpuBuffSz);
                buffLen = cpuBuffSz;
                [obj->buff release];
                obj->buff = buff;
                buffIsNew = ScnTRUE;
            }
        }
        //sync
        if(buffLen < cpuBuffSz){
            SCN_PRINTF_ERROR("ScnApiMetal_buffer_sync::gpuBuff is smaller than cpu-buff.\n");
        } else if(buffIsNew || changes->all){
            //sync all
            const STScnRangeU rngAll = ScnMemElastic_getUsedAddressesRngAligned(mem);
            if(!ScnApiMetal_buffer_syncRanges_(obj->buff, mem, &rngAll, 1)){
                SCN_PRINTF_ERROR("ScnApiMetal_buffer_sync::ScnApiMetal_buffer_syncRanges_ failed.\n");
            } else {
                r = ScnTRUE;
                //validate
                //SCN_ASSERT(ScnApiMetal_buffer_syncValidate_(obj, mem))
            }
        } else {
            //sync ranges only
            if(!ScnApiMetal_buffer_syncRanges_(obj->buff, mem, changes->rngs, changes->rngsUse)){
                SCN_PRINTF_ERROR("ScnApiMetal_buffer_sync::ScnApiMetal_buffer_syncRanges_ failed.\n");
            } else {
                r = ScnTRUE;
                //validate
                //SCN_ASSERT(ScnApiMetal_buffer_syncValidate_(obj, mem))
            }
        }
    }
    return r;
}

#ifdef SCN_ASSERTS_ACTIVATED
typedef struct ScnApiMetal_buffer_syncValidate_st {
    STScnApiMetalBuffer*    obj;
    STScnRangeU             usedAddressesRng;
    ScnUI32                 leftLimitFnd;
    ScnUI32                 rghtLimitFnd;
} ScnApiMetal_buffer_syncValidate_st;
#endif

#ifdef SCN_ASSERTS_ACTIVATED
ScnBOOL ScnApiMetal_buffer_syncValidate_pushBlockPtrs_(void* data, const ScnUI32 rootIndex, const void* rootAddress, const STScnMemBlockPtr* const ptrs, const ScnUI32 ptrsSz){
    ScnBOOL r = ScnFALSE;
    ScnApiMetal_buffer_syncValidate_st* st = (ScnApiMetal_buffer_syncValidate_st*)data;
    STScnApiMetalBuffer* obj = st->obj;
    const STScnRangeU usedAddressesRng = st->usedAddressesRng;
    if(obj->buff != nil){
        const ScnUI32 buffLen       = (ScnUI32)obj->buff.length;
        ScnBYTE* buffPtr            = (ScnBYTE*)obj->buff.contents;
        const STScnMemBlockPtr* ptr = ptrs;
        const STScnMemBlockPtr* ptrAfterEnd = ptr + ptrsSz;
        const ScnUI32 usedAddressesRngAfterEnd = usedAddressesRng.start + usedAddressesRng.size;
        r = ScnTRUE;
        while(ptr < ptrAfterEnd){
            const ScnUI32 ptrIdx = rootIndex + (ScnUI32)((const ScnBYTE*)ptr->ptr - (const ScnBYTE*)rootAddress);
            const ScnUI32 ptrIdxAfterEnd = ptrIdx + ptr->sz;
            if(ptrIdx < usedAddressesRng.start || ptrIdxAfterEnd > usedAddressesRngAfterEnd){
                SCN_ASSERT(ScnFALSE) //pointer out of ScnMemElastic's declared acitve range (program logic error)
                r = ScnFALSE;
                break;
            } else if(ptrIdxAfterEnd > buffLen){
                SCN_ASSERT(ScnFALSE) //pointer out of buffer's range
                r = ScnFALSE;
                break;
            } else if(ptr->sz > 0 && 0 != strncmp((const char*)&buffPtr[ptrIdx], ptr->ptr, ptr->sz)){
                SCN_ASSERT(ScnFALSE) //data missmatch
                r = ScnFALSE;
                break;
            }
            if(ptrIdx <= st->leftLimitFnd){
                SCN_ASSERT(ptrIdx < st->leftLimitFnd) //ptrs should not repeat themself
                st->leftLimitFnd = ptrIdx;
            }
            if(ptrIdxAfterEnd >= st->rghtLimitFnd){
                SCN_ASSERT(ptrIdxAfterEnd > st->rghtLimitFnd) //ptrs should not repeat themself
                st->rghtLimitFnd = ptrIdxAfterEnd;
            }
            //next
            ++ptr;
        }
    }
    return r;
}
#endif

#ifdef SCN_ASSERTS_ACTIVATED
ScnBOOL ScnApiMetal_buffer_syncValidate_(STScnApiMetalBuffer* obj, ScnMemElasticRef mem){
    ScnBOOL r = ScnTRUE;
    ScnApiMetal_buffer_syncValidate_st st;
    STScnMemPushPtrsItf itf;
    ScnMemory_setZeroSt(itf);
    ScnMemory_setZeroSt(st);
    itf.pushBlockPtrs   = ScnApiMetal_buffer_syncValidate_pushBlockPtrs_;
    st.obj              = obj;
    st.usedAddressesRng = ScnMemElastic_getUsedAddressesRng(mem);
    st.leftLimitFnd     = 0xFFFFFFFFu;
    st.rghtLimitFnd     = 0;
    {
        r = ScnMemElastic_pushPtrs(mem, &itf, &st);
    }
    SCN_ASSERT(st.leftLimitFnd == st.usedAddressesRng.start && st.rghtLimitFnd == (st.usedAddressesRng.start + st.usedAddressesRng.size)) //program logic error, the used-address-rng notified by the elastic-memory was miscalculated
    return r;
}
#endif

ScnBOOL ScnApiMetal_buffer_syncRanges_(id<MTLBuffer> buff, ScnMemElasticRef mem, const STScnRangeU* const rngs, const ScnUI32 rngsUse){
    ScnBOOL r = ScnTRUE;
    if(rngsUse <= 0){
        //nothing to sync
        return r;
    }
    //
    const ScnUI32 buffLen = (ScnUI32)buff.length;
    ScnBYTE* buffPtr = (ScnBYTE*)buff.contents;
    STScnAbsPtr ptr = STScnAbsPtr_Zero;
    ScnUI32 continuousSz = 0, copySz = 0, bytesCopied = 0;
    //
#   ifdef SCN_ASSERTS_ACTIVATED
    ScnUI32 prevRngAfterEnd = 0;
#   endif
    const STScnRangeU* rng = rngs;
    const STScnRangeU* rngAfterEnd = rngs + rngsUse;
    STScnRangeU curRng;
    while(rng < rngAfterEnd && r){
        SCN_ASSERT(prevRngAfterEnd <= rng->start + rng->size) //rngs should be ordered and non-overlapping
        //copy range data
        curRng = *rng;
        while(curRng.size > 0){
            ptr = ScnMemElastic_getNextContinuousAddress(mem, curRng.start, &continuousSz);
            if(ptr.ptr == NULL){
                break;
            }
            SCN_ASSERT(ptr.idx == curRng.start);
            SCN_ASSERT((curRng.start + continuousSz) <= buffLen);
            if((curRng.start + continuousSz) > buffLen){
                SCN_PRINTF_ERROR("gpu-buffer is smaller than cpu-buffer.\n");
                r = ScnFALSE;
                break;
            }
            //copy
            copySz = (curRng.size < continuousSz ? curRng.size : continuousSz);
            ScnMemcpy(&buffPtr[curRng.start], ptr.ptr, copySz);
            bytesCopied += copySz;
            //
            SCN_ASSERT(curRng.size >= copySz)
            curRng.start += copySz;
            curRng.size -= copySz;
        }
#       ifdef SCN_ASSERTS_ACTIVATED
        prevRngAfterEnd = rng->start + rng->size;
#       endif
        //next
        ++rng;
    }
    SCN_PRINTF_VERB("%2.f%% %u of %u bytes synced at buffer.\n", (float)bytesCopied * 100.f / (float)buffLen, bytesCopied, buffLen);
    return r;
}

//STScnApiMetalVertexBuffer

typedef struct STScnApiMetalVertexBuff {
    ScnContextRef           ctx;
    STScnGpuVertexbuffCfg   cfg;
    STScnApiItf             itf;
    ScnGpuBufferRef         vBuff;
    ScnGpuBufferRef         idxBuff;
} STScnApiMetalVertexBuff;

ScnGpuVertexbuffRef ScnApiMetal_device_allocVertexBuff(void* pObj, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff){
    ScnGpuVertexbuffRef r = ScnObjRef_Zero;
    STScnApiMetalDevice* dev = (STScnApiMetalDevice*)pObj;
    if(dev != NULL && dev->dev != NULL && cfg != NULL && !ScnGpuBuffer_isNull(vBuff)){ //idxBuff is optional
        //synced
        STScnApiMetalVertexBuff* obj = (STScnApiMetalVertexBuff*)ScnContext_malloc(dev->ctx, sizeof(STScnApiMetalVertexBuff), SCN_DBG_STR("STScnApiMetalVertexBuff"));
        if(obj == NULL){
            SCN_PRINTF_ERROR("ScnContext_malloc(STScnApiMetalVertexBuff) failed.\n");
        } else {
            ScnMemory_setZeroSt(*obj);
            ScnContext_set(&obj->ctx, dev->ctx);
            obj->itf        = dev->itf;
            obj->cfg        = *cfg;
            ScnGpuBuffer_set(&obj->vBuff, vBuff);
            ScnGpuBuffer_set(&obj->idxBuff, idxBuff);
            //
            ScnGpuVertexbuffRef d = ScnGpuVertexbuff_alloc(dev->ctx);
            if(!ScnGpuVertexbuff_isNull(d)){
                if(!ScnGpuVertexbuff_prepare(d, &obj->itf.vertexBuff, obj)){
                    SCN_PRINTF_ERROR("ScnApiMetal_device_allocVertexBuff::ScnGpuVertexbuff_prepare failed.\n");
                } else {
                    ScnGpuVertexbuff_set(&r, d);
                    obj = NULL; //consume
                }
                ScnGpuVertexbuff_releaseAndNull(&d);
            }
        }
        //release (if not consumed)
        if(obj != NULL){
            ScnApiMetal_vertexbuff_free(obj);
            obj = NULL;
        }
    }
    return r;
}

void ScnApiMetal_vertexbuff_free(void* pObj){
    STScnApiMetalVertexBuff* obj = (STScnApiMetalVertexBuff*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        ScnGpuBuffer_releaseAndNull(&obj->vBuff);
        ScnGpuBuffer_releaseAndNull(&obj->idxBuff);
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

ScnBOOL ScnApiMetal_vertexbuff_sync(void* pObj, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff){
    ScnBOOL r = ScnFALSE;
    STScnApiMetalVertexBuff* obj = (STScnApiMetalVertexBuff*)pObj;
    {
        obj->cfg = *cfg;
        ScnGpuBuffer_set(&obj->vBuff, vBuff);
        ScnGpuBuffer_set(&obj->idxBuff, idxBuff);
        //ToDo: implement
        r = ScnTRUE;
    }
    return r;
}

ScnBOOL ScnApiMetal_vertexbuff_activate(void* pObj){
    ScnBOOL r = ScnFALSE;
    //STScnApiMetalVertexBuff* obj = (STScnApiMetalVertexBuff*)pObj;
    {
        //ToDo: implement
        r = ScnTRUE;
    }
    return r;
}

ScnBOOL ScnApiMetal_vertexbuff_deactivate(void* pObj){
    ScnBOOL r = ScnFALSE;
    //STScnApiMetalVertexBuff* obj = (STScnApiMetalVertexBuff*)pObj;
    {
        //ToDo: implement
        r = ScnTRUE;
    }
    return r;
}

// STScnApiMetalTexture

typedef struct STScnApiMetalTexture {
    ScnContextRef           ctx;
    STScnGpuTextureCfg      cfg;
    id<MTLTexture>          tex;
    ScnGpuSamplerRef        sampler;
    STScnApiItf             itf;
} STScnApiMetalTexture;

ScnGpuTextureRef ScnApiMetal_device_allocTexture(void* pObj, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData){
    ScnGpuTextureRef r = ScnObjRef_Zero;
    STScnApiMetalDevice* dev = (STScnApiMetalDevice*)pObj;
    if(dev != NULL && dev->dev != nil && cfg != NULL){
        MTLPixelFormat fmt = MTLPixelFormatInvalid;
        switch (cfg->color) {
            case ENScnBitmapColor_ALPHA8: fmt = MTLPixelFormatA8Unorm; break;
            case ENScnBitmapColor_GRAY8: fmt = MTLPixelFormatR8Unorm; break;
            case ENScnBitmapColor_GRAYALPHA8: fmt = MTLPixelFormatRG8Unorm; break;
            case ENScnBitmapColor_RGBA8: fmt = MTLPixelFormatRGBA8Unorm; break;
            default: break;
        }
        if(fmt == MTLPixelFormatInvalid){
            SCN_PRINTF_ERROR("unsupported texture color format(%d).\n", cfg->color);
        } else if(cfg->width <= 0 && cfg->height <= 0){
            SCN_PRINTF_ERROR("invalid texture size(%d, %d).\n", cfg->width, cfg->height);
        } else {
            STScnApiMetalTexture* obj = NULL;
            MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
            id<MTLTexture> tex = nil;
            //
            texDesc.pixelFormat = fmt;
            texDesc.width       = cfg->width;
            texDesc.height      = cfg->height;
            //
            tex = [dev->dev newTextureWithDescriptor:texDesc];
            if(tex == nil){
                SCN_PRINTF_ERROR("newTextureWithDescriptor failed.\n");
            } else if(srcProps != NULL && (srcProps->size.width != cfg->width || srcProps->size.height != cfg->height || srcProps->color != cfg->color || srcProps->bytesPerLine <= 0 || srcData == NULL)){
                SCN_PRINTF_ERROR("texture and source props missmatch.\n");
            } else if(NULL == (obj = (STScnApiMetalTexture*)ScnContext_malloc(dev->ctx, sizeof(STScnApiMetalTexture), SCN_DBG_STR("STScnApiMetalTexture")))){
                SCN_PRINTF_ERROR("ScnContext_malloc(STScnApiMetalTexture) failed.\n");
            } else {
                ScnMemory_setZeroSt(*obj);
                ScnContext_set(&obj->ctx, dev->ctx);
                obj->itf            = dev->itf;
                obj->tex            = tex; [obj->tex retain];
                obj->cfg            = *cfg;
                obj->sampler        = ScnApiMetal_device_allocSampler(dev, &cfg->sampler);
                if(ScnGpuSampler_isNull(obj->sampler)){
                    SCN_PRINTF_ERROR("ScnApiMetal_device_allocTexture::ScnApiMetal_device_allocSampler failed.\n");
                } else {
                    ScnGpuTextureRef d = ScnGpuTexture_alloc(dev->ctx);
                    if(!ScnGpuTexture_isNull(d)){
                        STScnGpuTextureApiItf itf;
                        ScnMemory_setZeroSt(itf);
                        itf.free        = ScnApiMetal_texture_free;
                        itf.sync        = ScnApiMetal_texture_sync;
                        if(!ScnGpuTexture_prepare(d, &itf, obj)){
                            SCN_PRINTF_ERROR("ScnApiMetal_device_allocTexture::ScnGpuTexture_prepare failed.\n");
                        } else {
                            //apply data
                            if(srcProps != NULL && srcData != NULL){
                                MTLRegion region = { { 0, 0, 0 }, {cfg->width, cfg->height, 1} };
                                [tex replaceRegion:region mipmapLevel:0 withBytes:srcData bytesPerRow:srcProps->bytesPerLine];
                            }
                            //
                            ScnGpuTexture_set(&r, d);
                            obj = NULL; //consume
                        }
                        ScnGpuTexture_releaseAndNull(&d);
                    }
                }
            }
            //
            if(texDesc != nil){ [texDesc release]; texDesc = nil; }
            if(tex != nil){ [tex release]; tex = nil; }
            if(obj != NULL){
                ScnApiMetal_texture_free(obj);
                obj = NULL;
            }
        }
    }
    return r;
}

void ScnApiMetal_texture_free(void* pObj){
    STScnApiMetalTexture* obj = (STScnApiMetalTexture*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        if(obj->tex != nil){
            [obj->tex release];
            obj->tex = nil;
        }
        ScnGpuSampler_releaseAndNull(&obj->sampler);
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

ScnBOOL ScnApiMetal_texture_sync(void* pObj, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData, const STScnGpuTextureChanges* const changes){
    ScnBOOL r = ScnFALSE;
    STScnApiMetalTexture* obj = (STScnApiMetalTexture*)pObj;
    if(!(obj != NULL && obj->tex != NULL && srcProps != NULL && srcData != NULL && changes != NULL)){
        //missing params or objects
        return r;
    }
    if(!(obj->cfg.width == cfg->width && obj->cfg.height == cfg->height && obj->cfg.color == cfg->color)){
        //change of size of color is not supported
        return r;
    }
    //sync
    {
        ScnUI32 pxUpdated = 0, lnsUpdated = 0;
        if(changes->all){
            //update the texture
            MTLRegion region = { { 0, 0, 0 }, {obj->cfg.width, obj->cfg.height, 1} };
            [obj->tex replaceRegion:region mipmapLevel:0 withBytes:srcData bytesPerRow:srcProps->bytesPerLine];
            pxUpdated   += (obj->cfg.width * obj->cfg.height);
            lnsUpdated  += obj->cfg.height;
            r = ScnTRUE;
        } else if(changes->rngsUse == 0){
            //nothing to sync
            r = ScnTRUE;
        } else if(changes->rngs != 0){
#           ifdef SCN_ASSERTS_ACTIVATED
            ScnUI32 prevRngAfterEnd = 0;
#           endif
            const STScnRangeU* rng = changes->rngs;
            const STScnRangeU* rngAfterEnd = rng + changes->rngsUse;
            r = ScnTRUE;
            while(rng < rngAfterEnd){
                SCN_ASSERT(prevRngAfterEnd <= rng->start + rng->size) //rngs should be ordered and non-overlapping
                if(rng->start > obj->cfg.height || (rng->start + rng->size) > obj->cfg.height){
                    //out of range
                    r = ScnFALSE;
                    break;
                }
                //update the texture area
                {
                    MTLRegion region = { { 0, rng->start, 0 }, {obj->cfg.width, rng->size, 1} };
                    const void* srcRow = ((ScnBYTE*)srcData) + (rng->start * srcProps->bytesPerLine);
                    [obj->tex replaceRegion:region mipmapLevel:0 withBytes:srcRow bytesPerRow:srcProps->bytesPerLine];
                    pxUpdated   += (obj->cfg.width * rng->size);
                    lnsUpdated  += rng->size;
                }
#               ifdef SCN_ASSERTS_ACTIVATED
                prevRngAfterEnd = rng->start + rng->size;
#               endif
                ++rng;
            }
            r = ScnTRUE;
        }
        SCN_PRINTF_VERB("%2.f%% %u of %u lines synced at texture (%.1f Kpixs, %.1f KBs).\n", (float)lnsUpdated * 100.f / (float)obj->cfg.height, lnsUpdated, obj->cfg.height, (ScnFLOAT)pxUpdated / 1024.f, (ScnFLOAT)lnsUpdated * (ScnFLOAT)srcProps->bytesPerLine / 1024.f);
    }
    return r;
}

//STScnApiMetalRenderStates

#define ScnApiMetalRenderStates_ENScnVertexTypeFromVBuffCfg(VBUFF_CFG)  (VBUFF_CFG->texCoords[ENScnGpuTextureIdx_2].amm > 0 ? ENScnVertexType_2DTex3 : VBUFF_CFG->texCoords[ENScnGpuTextureIdx_1].amm > 0 ? ENScnVertexType_2DTex2 : VBUFF_CFG->texCoords[ENScnGpuTextureIdx_0].amm > 0 ? ENScnVertexType_2DTex : ENScnVertexType_2DColor)

typedef struct STScnApiMetalRenderStates {
    MTLPixelFormat              color;
    id<MTLRenderPipelineState>  states[ENScnVertexType_Count]; //shader and fragment for this framebuffer
} STScnApiMetalRenderStates;

void    STScnApiMetalRenderStates_init(STScnApiMetalRenderStates* obj);
void    STScnApiMetalRenderStates_destroy(STScnApiMetalRenderStates* obj);
ScnBOOL STScnApiMetalRenderStates_load(STScnApiMetalRenderStates* obj, STScnApiMetalDevice* dev, MTLPixelFormat color);

//STScnApiMetalFramebuffView

typedef struct STScnApiMetalFramebuffView {
    ScnContextRef           ctx;
    MTKView*                mtkView;
    STScnSize2DU            size;
    STScnGpuFramebuffProps  props;
    STScnApiItf             itf;
    STScnApiMetalRenderStates rndrShaders; //shaders
    //cur (state while sending commands)
    struct {
        //verts
        struct {
            ENScnVertexType type;
            STScnApiMetalBuffer* buff;
            STScnApiMetalBuffer* idxs;
        } verts;
    } cur;
} STScnApiMetalFramebuffView;

ScnGpuFramebuffRef ScnApiMetal_device_allocFramebuffFromOSView(void* pObj, void* pMtkView){
    ScnGpuFramebuffRef r = ScnObjRef_Zero;
    STScnApiMetalDevice* dev = (STScnApiMetalDevice*)pObj;
    MTKView* mtkView = (MTKView*)pMtkView;
    if(dev != NULL && dev->dev != NULL && mtkView != nil){
        STScnApiMetalFramebuffView* obj = NULL;
        STScnApiMetalRenderStates rndrShaders;
        STScnApiMetalRenderStates_init(&rndrShaders);
        if(!STScnApiMetalRenderStates_load(&rndrShaders, dev, mtkView.colorPixelFormat)){
            SCN_PRINTF_ERROR("STScnApiMetalRenderStates_load failed.\n");
        } else if(NULL == (obj = (STScnApiMetalFramebuffView*)ScnContext_malloc(dev->ctx, sizeof(STScnApiMetalFramebuffView), SCN_DBG_STR("STScnApiMetalFramebuffView")))){
            SCN_PRINTF_ERROR("ScnContext_malloc(STScnApiMetalFramebuffView) failed.\n");
            STScnApiMetalRenderStates_destroy(&rndrShaders);
        } else {
            CGSize viewSz = mtkView.drawableSize;
            ScnMemory_setZeroSt(*obj);
            ScnContext_set(&obj->ctx, dev->ctx);
            obj->itf            = dev->itf;
            obj->mtkView        = mtkView; [obj->mtkView retain];
            obj->rndrShaders   = rndrShaders;
            {
                //size
                obj->size.width             = viewSz.width;
                obj->size.height            = viewSz.height;
                //viewport
                obj->props.viewport.x       = 0;
                obj->props.viewport.y       = 0;
                obj->props.viewport.width   = obj->size.width;
                obj->props.viewport.height  = obj->size.height;
                //ortho2d
                obj->props.ortho.x.min      = 0.f;
                obj->props.ortho.x.max      = obj->size.width;
                obj->props.ortho.y.min      = 0.f;
                obj->props.ortho.y.max      = obj->size.height;
            }
            //
            ScnGpuFramebuffRef d = ScnGpuFramebuff_alloc(dev->ctx);
            if(!ScnGpuFramebuff_isNull(d)){
                STScnGpuFramebuffApiItf itf;
                ScnMemory_setZeroSt(itf);
                itf.free        = ScnApiMetal_framebuff_view_free;
                itf.getSize     = ScnApiMetal_framebuff_view_getSize;
                itf.syncSize    = ScnApiMetal_framebuff_view_syncSize;
                itf.getProps    = ScnApiMetal_framebuff_view_getProps;
                itf.setProps    = ScnApiMetal_framebuff_view_setProps;
                if(!ScnGpuFramebuff_prepare(d, &itf, obj)){
                    SCN_PRINTF_ERROR("ScnApiMetal_device_allocFramebuffFromOSView::ScnGpuFramebuff_prepare failed.\n");
                } else {
                    //configure view
                    mtkView.device = dev->dev;
                    //
                    ScnGpuFramebuff_set(&r, d);
                    obj = NULL; //consume
                }
                ScnGpuFramebuff_releaseAndNull(&d);
            }
        }
        //release (if not consumed)
        if(obj != NULL){
            ScnApiMetal_framebuff_view_free(obj);
            obj = NULL;
        }
    }
    return r;
}

//frameBuffer (view)

void ScnApiMetal_framebuff_view_free(void* pObj){
    STScnApiMetalFramebuffView* obj = (STScnApiMetalFramebuffView*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        //
        STScnApiMetalRenderStates_destroy(&obj->rndrShaders);
        //
        if(obj->mtkView != nil){
            [obj->mtkView release];
            obj->mtkView = nil;
        }
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

STScnSize2DU ScnApiMetal_framebuff_view_getSize(void* pObj){
    STScnApiMetalFramebuffView* obj = (STScnApiMetalFramebuffView*)pObj;
    return obj->size;
}

ScnBOOL ScnApiMetal_framebuff_view_syncSize(void* pObj, const STScnSize2DU size){
    ScnBOOL r = ScnFALSE;
    STScnApiMetalFramebuffView* obj = (STScnApiMetalFramebuffView*)pObj;
    if(obj->mtkView != nil){
        //const CGSize sz = obj->mtkView.drawableSize;
        //obj->size.width = sz.width;
        //obj->size.height = sz.height;
        obj->size = size;
        r = ScnTRUE;
    }
    return r;
}

STScnGpuFramebuffProps ScnApiMetal_framebuff_view_getProps(void* pObj){
    STScnApiMetalFramebuffView* obj = (STScnApiMetalFramebuffView*)pObj;
    return obj->props;
}

ScnBOOL ScnApiMetal_framebuff_view_setProps(void* pObj, const STScnGpuFramebuffProps* const props){
    ScnBOOL r = ScnFALSE;
    STScnApiMetalFramebuffView* obj = (STScnApiMetalFramebuffView*)pObj;
    if(obj->mtkView != nil && props != NULL){
        obj->props = *props;
        r = ScnTRUE;
    }
    return r;
}

// STScnApiMetalRenderStates

void STScnApiMetalRenderStates_init(STScnApiMetalRenderStates* obj){
    ScnMemory_setZeroSt(*obj);
}

void STScnApiMetalRenderStates_destroy(STScnApiMetalRenderStates* obj){
    //states
    {
        id<MTLRenderPipelineState>* s = &obj->states[0];
        id<MTLRenderPipelineState>* sAfterEnd = s + (sizeof(obj->states) / sizeof(obj->states[0]));
        while(s < sAfterEnd){
            if(*s != nil){
                [*s release];
                *s  = nil;
            }
            ++s;
        }
    }
}

//

ScnBOOL STScnApiMetalRenderStates_load(STScnApiMetalRenderStates* obj, STScnApiMetalDevice* dev, MTLPixelFormat color){
    ScnBOOL r = ScnTRUE;
    obj->color = color;
    {
        id<MTLFunction> vertexFunc = nil;
        id<MTLFunction> fragmtFunc = nil;
        ScnUI32 i; for(i = 0; i < (sizeof(obj->states) / sizeof(obj->states[0])); ++i){
            const char* vertexFuncName = NULL;
            const char* fragmtFuncName = NULL;
            switch (i) {
                case ENScnVertexType_2DColor: vertexFuncName = "ixtliVertexShader"; fragmtFuncName = "ixtliFragmentShader"; break;
                case ENScnVertexType_2DTex:   vertexFuncName = "ixtliVertexShaderTex"; fragmtFuncName = "ixtliFragmentShaderTex"; break;
                case ENScnVertexType_2DTex2:  vertexFuncName = "ixtliVertexShaderTex2"; fragmtFuncName = "ixtliFragmentShaderTex2"; break;
                case ENScnVertexType_2DTex3:  vertexFuncName = "ixtliVertexShaderTex3"; fragmtFuncName = "ixtliFragmentShaderTex3"; break;
                default: break;
            }
            if(vertexFuncName == NULL){
                SCN_PRINTF_ERROR("STScnApiMetalRenderStates_load unimplemented function name.\n");
                r = ScnFALSE;
                break;
            }
            if(fragmtFuncName == NULL){
                SCN_PRINTF_ERROR("STScnApiMetalRenderStates_load unimplemented function name.\n");
                r = ScnFALSE;
                break;
            }
            //
            if(vertexFunc != nil){ [vertexFunc release]; vertexFunc = nil; }
            if(fragmtFunc != nil){ [fragmtFunc release]; fragmtFunc = nil; }
            //
            vertexFunc = [dev->lib newFunctionWithName:[NSString stringWithUTF8String:vertexFuncName]];
            if(vertexFunc == nil){
                SCN_PRINTF_ERROR("STScnApiMetalRenderStates_load newFunctionWithName('%s') failed.\n", vertexFuncName);
                r = ScnFALSE;
                break;
            }
            fragmtFunc = [dev->lib newFunctionWithName:[NSString stringWithUTF8String:fragmtFuncName]];
            if(fragmtFunc == nil){
                SCN_PRINTF_ERROR("STScnApiMetalRenderStates_load newFunctionWithName('%s') failed.\n", fragmtFuncName);
                r = ScnFALSE;
                break;
            }
            //create state
            {
                NSError *error;
                MTLRenderPipelineDescriptor* rndrPipeDesc = [[MTLRenderPipelineDescriptor alloc] init];
                rndrPipeDesc.label = @"Ixtla-render default (fixed) render pipeline.";
                rndrPipeDesc.vertexFunction = vertexFunc; [vertexFunc retain];
                rndrPipeDesc.fragmentFunction = fragmtFunc; [fragmtFunc retain];
                rndrPipeDesc.colorAttachments[0].pixelFormat = color;
                rndrPipeDesc.colorAttachments[0].blendingEnabled = YES;
                rndrPipeDesc.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
                rndrPipeDesc.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
                rndrPipeDesc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
                rndrPipeDesc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
                rndrPipeDesc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
                rndrPipeDesc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
                if(nil == (obj->states[i] = [dev->dev newRenderPipelineStateWithDescriptor:rndrPipeDesc error:&error])){
                    SCN_PRINTF_ERROR("STScnApiMetalRenderStates_load::newRenderPipelineStateWithDescriptor failed: '%s'.\n", (error == nil ? "nil" : [[error description] UTF8String]));
                    r = ScnFALSE;
                    break;
                }
            }
        }
        if(vertexFunc != nil){ [vertexFunc release]; vertexFunc = nil; }
        if(fragmtFunc != nil){ [fragmtFunc release]; fragmtFunc = nil; }
    }
    return r;
}

// STScnApiMetalRenderJobState

typedef struct STScnApiMetalRenderJobState {
    STScnApiMetalFramebuffView* fb;
    MTLRenderPassDescriptor*    rndrDesc; //per active framebuff
    id<MTLRenderCommandEncoder> rndrEnc; //per active framebuff
} STScnApiMetalRenderJobState;

void ScnApiMetalRenderJobState_init(STScnApiMetalRenderJobState* obj);
void ScnApiMetalRenderJobState_destroy(STScnApiMetalRenderJobState* obj);
//
void ScnApiMetalRenderJobState_reset(STScnApiMetalRenderJobState* obj);

// STScnApiMetalRenderJob

typedef struct STScnApiMetalRenderJob {
    ScnContextRef           ctx;
    STScnApiMetalDevice*    dev;
    //bPropsScns
    struct {
        ScnGpuBufferRef     ref;
        STScnApiMetalBuffer* obj;
    } bPropsScns;
    //bPropsMdls
    struct {
        ScnGpuBufferRef     ref;
        STScnApiMetalBuffer* obj;
    } bPropsMdls;
    //
    id<MTLCommandBuffer>    cmdsBuff;
    //
    STScnApiMetalRenderJobState state;
} STScnApiMetalRenderJob;

ScnGpuRenderJobRef ScnApiMetal_device_allocRenderJob(void* pObj){
    ScnGpuRenderJobRef r = ScnGpuRenderJobRef_Zero;
    STScnApiMetalDevice* dev = (STScnApiMetalDevice*)pObj;
    if(dev != NULL && dev->dev != NULL && dev->cmdQueue != nil){
        STScnApiMetalRenderJob* obj = (STScnApiMetalRenderJob*)ScnContext_malloc(dev->ctx, sizeof(STScnApiMetalRenderJob), SCN_DBG_STR("STScnApiMetalRenderJob"));
        if(obj == NULL){
            SCN_PRINTF_ERROR("ScnContext_malloc(STScnApiMetalRenderJob) failed.\n");
        } else {
            ScnMemory_setZeroSt(*obj);
            ScnContext_set(&obj->ctx, dev->ctx);
            ScnApiMetalRenderJobState_init(&obj->state);
            obj->dev        = dev;  //retain?
            //
            ScnGpuRenderJobRef d = ScnGpuRenderJob_alloc(dev->ctx);
            if(!ScnGpuRenderJob_isNull(d)){
                //
                STScnGpuRenderJobApiItf itf;
                ScnMemory_setZeroSt(itf);
                itf.free        = ScnApiMetal_renderJob_free;
                itf.getState    = ScnApiMetal_renderJob_getState;
                itf.buildBegin  = ScnApiMetal_renderJob_buildBegin;
                itf.buildAddCmds = ScnApiMetal_renderJob_buildAddCmds;
                itf.buildEndAndEnqueue = ScnApiMetal_renderJob_buildEndAndEnqueue;
                //
                if(!ScnGpuRenderJob_prepare(d, &itf, obj)){
                    SCN_PRINTF_ERROR("ScnApiMetal_device_allocRenderJob::ScnGpuRenderJob_prepare failed.\n");
                } else {
                    ScnGpuRenderJob_set(&r, d);
                    obj = NULL; //consume
                }
                ScnGpuRenderJob_releaseAndNull(&d);
            }
        }
        //release (if not consumed)
        if(obj != NULL){
            ScnApiMetal_renderJob_free(obj);
            obj = NULL;
        }
    }
    return r;
}


//render job

void ScnApiMetal_renderJob_free(void* data){
    STScnApiMetalRenderJob* obj = (STScnApiMetalRenderJob*)data;
    ScnContextRef ctx = obj->ctx;
    {
        if(obj->cmdsBuff != nil){
#           ifdef SCN_ASSERTS_ACTIVATED
            const MTLCommandBufferStatus status = [obj->cmdsBuff status];
            SCN_ASSERT(status == MTLCommandBufferStatusNotEnqueued || status == MTLCommandBufferStatusCompleted || status == MTLCommandBufferStatusError) //should not be active
#           endif
            [obj->cmdsBuff release];
            obj->cmdsBuff = nil;
        }
        //bPropsScns
        {
            ScnGpuBuffer_releaseAndNull(&obj->bPropsScns.ref);
            obj->bPropsScns.obj = NULL;
        }
        //bPropsMdls
        {
            ScnGpuBuffer_releaseAndNull(&obj->bPropsMdls.ref);
            obj->bPropsMdls.obj = NULL;
        }
        ScnApiMetalRenderJobState_destroy(&obj->state);
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

ENScnGpuRenderJobState ScnApiMetal_renderJob_getState(void* data){
    ENScnGpuRenderJobState r = ENScnGpuRenderJobState_Unknown;
    STScnApiMetalRenderJob* obj = (STScnApiMetalRenderJob*)data;
    if(obj->cmdsBuff != nil){
        switch ([obj->cmdsBuff status]) {
            case MTLCommandBufferStatusNotEnqueued: ; break;
            case MTLCommandBufferStatusEnqueued: r = ENScnGpuRenderJobState_Enqueued; break;
            case MTLCommandBufferStatusCommitted: r = ENScnGpuRenderJobState_Enqueued; break;
            case MTLCommandBufferStatusScheduled: r = ENScnGpuRenderJobState_Enqueued; break;
            case MTLCommandBufferStatusCompleted: r = ENScnGpuRenderJobState_Completed; break;
            case MTLCommandBufferStatusError: r = ENScnGpuRenderJobState_Error; break;
            default: break;
        }
    }
    return r;
}

ScnBOOL ScnApiMetal_renderJob_buildBegin(void* data, ScnGpuBufferRef pBuffPropsScns, ScnGpuBufferRef pBuffPropsMdls){
    ScnBOOL r = ScnFALSE;
    STScnApiMetalRenderJob* obj = (STScnApiMetalRenderJob*)data;
    //
    STScnApiMetalBuffer* bPropsScns = NULL;
    STScnApiMetalBuffer* bPropsMdls = NULL;
    //
    if(obj->dev == NULL || obj->dev->dev == nil || obj->dev->cmdQueue == nil){
        SCN_PRINTF_ERROR("ScnApiMetal_renderJob_buildBegin::dev-or-cmdQueue is NULL.\n");
        return ScnFALSE;
    }
    if(ScnGpuBuffer_isNull(pBuffPropsScns) || ScnGpuBuffer_isNull(pBuffPropsMdls)){
        //SCN_PRINTF_ERROR("ScnApiMetal_renderJob_buildBegin::pBuffPropsScns-or-pBuffPropsMdls is NULL.\n");
        return ScnFALSE;
    }
    bPropsScns = (STScnApiMetalBuffer*)ScnGpuBuffer_getApiItfParam(pBuffPropsScns);
    bPropsMdls = (STScnApiMetalBuffer*)ScnGpuBuffer_getApiItfParam(pBuffPropsMdls);
    if(bPropsScns == NULL || bPropsScns->buff == nil){
        SCN_PRINTF_ERROR("ScnApiMetal_renderJob_buildBegin::bPropsScns is NULL.\n");
        return ScnFALSE;
    } else if(bPropsMdls == NULL || bPropsMdls->buff == nil){
        SCN_PRINTF_ERROR("ScnApiMetal_renderJob_buildBegin::bPropsMdls is NULL.\n");
        return ScnFALSE;
    }
    if(obj->cmdsBuff != nil){
        const MTLCommandBufferStatus status = [obj->cmdsBuff status];
        if(status != MTLCommandBufferStatusNotEnqueued && status != MTLCommandBufferStatusCompleted && status != MTLCommandBufferStatusError){
            //job currentyl in progress
            SCN_PRINTF_ERROR("ScnApiMetal_renderJob_buildBegin::[obj->cmdsBuff status] is active.\n");
            return ScnFALSE;
        }
    }
    //begin
    {
        if(obj->cmdsBuff != nil){
            [obj->cmdsBuff release];
            obj->cmdsBuff = nil;
        }
        obj->cmdsBuff = [obj->dev->cmdQueue commandBuffer];
        if(obj->cmdsBuff != nil){
            obj->cmdsBuff.label = @"Ixtli-cmd-buff";
            [obj->cmdsBuff retain];
            {
                ScnGpuBuffer_set(&obj->bPropsScns.ref, pBuffPropsScns);
                obj->bPropsScns.obj = bPropsScns;
            }
            {
                ScnGpuBuffer_set(&obj->bPropsMdls.ref, pBuffPropsMdls);
                obj->bPropsMdls.obj = bPropsMdls;
            }
            ScnApiMetalRenderJobState_reset(&obj->state);
            //
            r = ScnTRUE;
        }
    }
    //
    return r;
    
}

ScnBOOL ScnApiMetal_renderJob_buildAddCmds(void* data, const struct STScnRenderCmd* const cmds, const ScnUI32 cmdsSz){
    ScnBOOL r = ScnFALSE;
    //
    STScnApiMetalRenderJob* obj = (STScnApiMetalRenderJob*)data;
    //
    if(obj->cmdsBuff == nil){
        return ScnFALSE;
    } else if(obj->bPropsScns.obj == NULL || obj->bPropsScns.obj->buff == nil){
        return ScnFALSE;
    } else if(obj->bPropsMdls.obj == NULL || obj->bPropsMdls.obj->buff == nil){
        return ScnFALSE;
    }
    //
    STScnApiMetalRenderJobState* state = &obj->state;
    const STScnRenderCmd* c = cmds;
    const STScnRenderCmd* cAfterEnd = cmds + cmdsSz;
    //
    r = ScnTRUE;
    //
    while(r && c < cAfterEnd){
        switch (c->cmdId) {
            case ENScnRenderCmd_None:
                //nop
                break;
                //framebuffers
            case ENScnRenderCmd_ActivateFramebuff:
                if(!ScnFramebuff_isNull(c->activateFramebuff.ref)){
                    ScnGpuFramebuffRef gpuFb = ScnFramebuff_getCurrentRenderSlot(c->activateFramebuff.ref);
                    if(ScnGpuFramebuff_isNull(gpuFb)){
                        SCN_PRINTF_ERROR("ENScnRenderCmd_ActivateFramebuff::ScnGpuFramebuff_isNull.\n");
                        r = ScnFALSE;
                    } else {
                        state->fb = (STScnApiMetalFramebuffView*)ScnGpuFramebuff_getApiItfParam(gpuFb);
                        if(state->fb == NULL || state->fb->mtkView == nil || state->fb->rndrShaders.states[state->fb->cur.verts.type] == nil){
                            SCN_PRINTF_ERROR("ENScnRenderCmd_ActivateFramebuff::fb == NULL || fb->mtkView == nil || fb->renderState == nil.\n");
                            r = ScnFALSE;
                            break;
                        }
                        //ToDo: define how to reset state
                        //ScnMemory_setZeroSt(fb->cur);
                        //
                        MTLRenderPassDescriptor* rndrDesc = state->fb->mtkView.currentRenderPassDescriptor;
                        SCN_ASSERT(obj->cmdsBuff != nil)
                        if(rndrDesc == nil){
                            SCN_PRINTF_ERROR("ENScnRenderCmd_ActivateFramebuff::renderPassDescriptor == nil || commandBuffer == nil.\n");
                            r = ScnFALSE;
                        } else {
                            id <MTLRenderCommandEncoder> rndrEnc = [obj->cmdsBuff renderCommandEncoderWithDescriptor:rndrDesc];
                            if(rndrEnc == nil){
                                SCN_PRINTF_ERROR("ENScnRenderCmd_ActivateFramebuff::renderCommandEncoderWithDescriptor failed.\n");
                                r = ScnFALSE;
                            } else {
                                if(state->rndrDesc != nil){ [state->rndrDesc release]; state->rndrDesc = nil; }
                                if(state->rndrEnc != nil){ [state->rndrEnc release]; state->rndrEnc = nil; }
                                //
                                state->rndrDesc = rndrDesc; [rndrDesc retain];
                                state->rndrEnc = rndrEnc; [rndrEnc retain];
                                state->rndrEnc.label = @"ixtli-render-cmd-enc";
                                //
                                [state->rndrEnc setRenderPipelineState:state->fb->rndrShaders.states[state->fb->cur.verts.type]];
                                //apply viewport
                                {
                                    MTLViewport viewPort;
                                    viewPort.originX = (double)c->activateFramebuff.viewport.x;
                                    viewPort.originY = (double)c->activateFramebuff.viewport.y;
                                    viewPort.width = (double)c->activateFramebuff.viewport.width;
                                    viewPort.height = (double)c->activateFramebuff.viewport.height;
                                    viewPort.znear = 0.0;
                                    viewPort.zfar = 1.0;
                                    [state->rndrEnc setViewport:viewPort];
                                    SCN_PRINTF_VERB("setViewport(%u, %u)-(+%u, +%u).\n", fb->viewport.x, fb->viewport.y, fb->viewport.width, fb->viewport.height);
                                }
                                //fb props
                                [state->rndrEnc setVertexBuffer:obj->bPropsScns.obj->buff offset:c->activateFramebuff.offset atIndex:0];
                                SCN_PRINTF_VERB("setVertexBuffer(idx0, %u offset).\n", c->activateFramebuff.offset);
                                //mdl props
                                [state->rndrEnc setVertexBuffer:obj->bPropsMdls.obj->buff offset:0 atIndex:1];
                                SCN_PRINTF_VERB("setVertexBuffer(idx1, 0 offset).\n");
                            }
                        }
                    }
                }
                break;
            case ENScnRenderCmd_SetFramebuffProps:
                if(state->rndrEnc != nil){
                    //apply viewport
                    {
                        MTLViewport viewPort;
                        viewPort.originX = (double)c->setFramebuffProps.viewport.x;
                        viewPort.originY = (double)c->setFramebuffProps.viewport.y;
                        viewPort.width = (double)c->setFramebuffProps.viewport.width;
                        viewPort.height = (double)c->setFramebuffProps.viewport.height;
                        viewPort.znear = 0.0;
                        viewPort.zfar = 1.0;
                        [state->rndrEnc setViewport:viewPort];
                        SCN_PRINTF_VERB("setViewport(%u, %u)-(+%u, +%u).\n", fb->viewport.x, fb->viewport.y, fb->viewport.width, fb->viewport.height);
                    }
                    //fb props
                    [state->rndrEnc setVertexBufferOffset:c->setFramebuffProps.offset atIndex:0];
                }
                break;
                //models
            case ENScnRenderCmd_SetTransformOffset: //sets the positions of the 'STScnGpuModelProps2d' to be applied for the drawing cmds
                if(state->rndrEnc == nil){
                    SCN_PRINTF_ERROR("ENScnRenderCmd_SetTransformOffset::rndrEnc is nil.\n");
                    r = ScnFALSE;
                } else {
                    [state->rndrEnc setVertexBufferOffset:c->setTransformOffset.offset atIndex:1];
                    SCN_PRINTF_VERB("setVertexBufferOffset(idx1, %u offset).\n", c->setTransformOffset.offset);
                }
                break;
            case ENScnRenderCmd_SetVertexBuff:  //activates the vertex buffer
                if(ScnVertexbuff_isNull(c->setVertexBuff.ref)){
                    SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::ScnVertexbuff_isNull.\n");
                    [state->rndrEnc setVertexBuffer:nil offset:0 atIndex:2];
                } else {
                    ScnGpuVertexbuffRef vbuffRef = ScnVertexbuff_getCurrentRenderSlot(c->setVertexBuff.ref);
                    if(ScnGpuVertexbuff_isNull(vbuffRef)){
                        SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::ScnGpuVertexbuff_isNull.\n");
                        r = ScnFALSE;
                    } else {
                        STScnApiMetalVertexBuff* vbuff = (STScnApiMetalVertexBuff*)ScnGpuVertexBuff_getApiItfParam(vbuffRef);
                        if(vbuff == NULL || ScnGpuBuffer_isNull(vbuff->vBuff)){
                            SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::ScnGpuBuffer_isNull.\n");
                            r = ScnFALSE;
                        } else {
                            STScnApiMetalBuffer* buff = (STScnApiMetalBuffer*)ScnGpuBuffer_getApiItfParam(vbuff->vBuff);
                            STScnApiMetalBuffer* idxs = ScnGpuBuffer_isNull(vbuff->idxBuff) ? NULL : (STScnApiMetalBuffer*)ScnGpuBuffer_getApiItfParam(vbuff->idxBuff);
                            if(buff == NULL){
                                SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::buff == NULL.\n");
                                r = ScnFALSE;
                            } else {
                                const ENScnVertexType vertexType = STScnGpuVertexbuffCfg_2_ENScnVertexType(&vbuff->cfg);
                                if(state->fb->rndrShaders.states[state->fb->cur.verts.type] == nil){
                                    SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::fb->rndrShaders.states[fb->curVertexType] == nil.\n");
                                    r = ScnFALSE;
                                } else {
                                    state->fb->cur.verts.type  = vertexType;
                                    state->fb->cur.verts.buff  = buff;
                                    state->fb->cur.verts.idxs  = idxs;
                                    //MTLRenderStageVertex   = (1UL << 0),
                                    //MTLRenderStageFragment = (1UL << 1),
                                    //if(c->setVertexBuff.isFirstUse){
                                    //    [state->rndrEnc useResource:buff->buff usage:MTLResourceUsageRead stages:MTLRenderStageVertex];
                                    //}
                                    [state->rndrEnc setRenderPipelineState:state->fb->rndrShaders.states[state->fb->cur.verts.type]];
                                    [state->rndrEnc setVertexBuffer:buff->buff offset:0 atIndex:2];
                                    SCN_PRINTF_VERB("setVertexBuffer(idx2, 0 offset).\n");
                                }
                            }
                        }
                    }
                }
                break;
           case ENScnRenderCmd_SetTexture:     //activates the texture in a specific slot-index
                if(ScnTexture_isNull(c->setTexture.ref)){
                    [state->rndrEnc setFragmentTexture:nil atIndex:c->setTexture.index];
                    [state->rndrEnc setFragmentSamplerState:nil atIndex:c->setTexture.index];
                } else {
                    ScnGpuTextureRef texRef = ScnTexture_getCurrentRenderSlot(c->setTexture.ref);
                    if(ScnGpuTexture_isNull(texRef)){
                        SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::ScnGpuTexture_isNull.\n");
                        r = ScnFALSE;
                    } else {
                        STScnApiMetalTexture* tex = (STScnApiMetalTexture*)ScnGpuTexture_getApiItfParam(texRef);
                        if(tex == NULL || tex->tex == nil || ScnGpuSampler_isNull(tex->sampler)){
                            SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::tex->tex is NULL.\n");
                            r = ScnFALSE;
                        } else {
                            STScnApiMetalSampler* smplr = (STScnApiMetalSampler*)ScnGpuSampler_getApiItfParam(tex->sampler);
                            if(smplr->smplr == nil){
                                SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::smplr->smplr is NULL.\n");
                                r = ScnFALSE;
                            } else {
                                //if(c->setTexture.isFirstUse){
                                //    [state->rndrEnc useResource:tex->tex usage:MTLResourceUsageRead stages:MTLRenderStageFragment];
                                //}
                                [state->rndrEnc setFragmentTexture:tex->tex atIndex:c->setTexture.index];
                                [state->rndrEnc setFragmentSamplerState:smplr->smplr atIndex:c->setTexture.index];
                            }
                        }
                    }
                }
                break;
                //modes
                //case ENScnRenderCmd_MaskModePush:   //pushes drawing-mask mode, where only the alpha value is affected
                //    break;
                //case ENScnRenderCmd_MaskModePop:    //pop
                //    break;
                //drawing
            case ENScnRenderCmd_DrawVerts:      //draws something using the vertices
                switch (c->drawVerts.shape) {
                    case ENScnRenderShape_Compute:
                        //nop
                        break;
                        //
                    case ENScnRenderShape_Texture:     //same as 'ENScnRenderShape_TriangStrip' with possible bitblit-optimization if matrix has no rotation.
                        if(state->rndrEnc == NULL){
                            //rintf("ERROR, ENScnRenderShape_Texture::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:c->drawVerts.iFirst vertexCount:c->drawVerts.count];
                            SCN_PRINTF_VERB("drawPrimitives(MTLPrimitiveTypeTriangleStrip: %u, +%u).\n", c->drawVerts.iFirst, c->drawVerts.count);
                        }
                        break;
                    case ENScnRenderShape_TriangStrip: //triangles-strip, most common shape
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_TriangStrip::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:c->drawVerts.iFirst vertexCount:c->drawVerts.count];
                            SCN_PRINTF_VERB("drawPrimitives(MTLPrimitiveTypeTriangleStrip: %u, +%u).\n", c->drawVerts.iFirst, c->drawVerts.count);
                        }
                        break;
                    //case ENScnRenderShape_TriangFan:   //triangles-fan
                    //    break;
                        //
                    case ENScnRenderShape_LineStrip:   //lines-strip
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_LineStrip::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawPrimitives:MTLPrimitiveTypeLineStrip vertexStart:c->drawVerts.iFirst vertexCount:c->drawVerts.count];
                            SCN_PRINTF_VERB("drawPrimitives(MTLPrimitiveTypeLineStrip: %u, +%u).\n", c->drawVerts.iFirst, c->drawVerts.count);
                        }
                        break;
                    //case ENScnRenderShape_LineLoop:    //lines-loop
                    //    break;
                    case ENScnRenderShape_Lines:       //lines
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_Lines::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawPrimitives:MTLPrimitiveTypeLine vertexStart:c->drawVerts.iFirst vertexCount:c->drawVerts.count];
                            SCN_PRINTF_VERB("drawPrimitives(MTLPrimitiveTypeLine: %u, +%u).\n", c->drawVerts.iFirst, c->drawVerts.count);
                        }
                        break;
                    case ENScnRenderShape_Points:      //points
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_Points::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawPrimitives:MTLPrimitiveTypePoint vertexStart:c->drawVerts.iFirst vertexCount:c->drawVerts.count];
                            SCN_PRINTF_VERB("drawPrimitives(MTLPrimitiveTypePoint: %u, +%u).\n", c->drawVerts.iFirst, c->drawVerts.count);
                        }
                        break;
                    default:
                        SCN_ASSERT(ScnFALSE); //missing implementation
                        break;
                }
                break;
            case ENScnRenderCmd_DrawIndexes:    //draws something using the vertices indexes
                if(state->fb == NULL || state->fb->cur.verts.idxs == NULL || state->fb->cur.verts.idxs->buff == nil){
                    SCN_PRINTF_ERROR("ENScnRenderCmd_DrawIndexes without active framebuffer or index-buffer.\n");
                    r = ScnFALSE;
                    break;
                }
                switch (c->drawIndexes.shape) {
                    case ENScnRenderShape_Compute:
                        //nop
                        break;
                        //
                    case ENScnRenderShape_Texture:     //same as 'ENScnRenderShape_TriangStrip' with possible bitblit-optimization if matrix has no rotation.
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_Texture::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawIndexedPrimitives:MTLPrimitiveTypeTriangleStrip indexCount:c->drawIndexes.count indexType:MTLIndexTypeUInt32 indexBuffer:state->fb->cur.verts.idxs->buff indexBufferOffset:c->drawIndexes.iFirst * 4];
                            SCN_PRINTF_VERB("drawIndexedPrimitives(MTLPrimitiveTypeTriangleStrip: %u, +%u).\n", c->drawIndexes.iFirst, c->drawIndexes.count);
                        }
                        break;
                    case ENScnRenderShape_TriangStrip: //triangles-strip, most common shape
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_TriangStrip::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawIndexedPrimitives:MTLPrimitiveTypeTriangleStrip indexCount:c->drawIndexes.count indexType:MTLIndexTypeUInt32 indexBuffer:state->fb->cur.verts.idxs->buff indexBufferOffset:c->drawIndexes.iFirst * 4];
                            SCN_PRINTF_VERB("drawIndexedPrimitives(MTLPrimitiveTypeTriangleStrip: %u, +%u).\n", c->drawIndexes.iFirst, c->drawIndexes.count);
                        }
                        break;
                    //case ENScnRenderShape_TriangFan:   //triangles-fan
                    //    break;
                        //
                    case ENScnRenderShape_LineStrip:   //lines-strip
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_LineStrip::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawIndexedPrimitives:MTLPrimitiveTypeLineStrip indexCount:c->drawIndexes.count indexType:MTLIndexTypeUInt32 indexBuffer:state->fb->cur.verts.idxs->buff indexBufferOffset:c->drawIndexes.iFirst * 4];
                            SCN_PRINTF_VERB("drawIndexedPrimitives(MTLPrimitiveTypeLineStrip: %u, +%u).\n", c->drawIndexes.iFirst, c->drawIndexes.count);
                        }
                        break;
                    //case ENScnRenderShape_LineLoop:    //lines-loop
                    //    break;
                    case ENScnRenderShape_Lines:       //lines
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_Lines::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawIndexedPrimitives:MTLPrimitiveTypeLine indexCount:c->drawIndexes.count indexType:MTLIndexTypeUInt32 indexBuffer:state->fb->cur.verts.idxs->buff indexBufferOffset:c->drawIndexes.iFirst * 4];
                            SCN_PRINTF_VERB("drawIndexedPrimitives(MTLPrimitiveTypeLine: %u, +%u).\n", c->drawIndexes.iFirst, c->drawIndexes.count);
                        }
                        break;
                    case ENScnRenderShape_Points:      //points
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_Points::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawIndexedPrimitives:MTLPrimitiveTypePoint indexCount:c->drawIndexes.count indexType:MTLIndexTypeUInt32 indexBuffer:state->fb->cur.verts.idxs->buff indexBufferOffset:c->drawIndexes.iFirst * 4];
                            SCN_PRINTF_VERB("drawIndexedPrimitives(MTLPrimitiveTypePoint: %u, +%u).\n", c->drawIndexes.iFirst, c->drawIndexes.count);
                        }
                        break;
                    default:
                        SCN_ASSERT(ScnFALSE); //missing implementation
                        break;
                }
                break;
            default:
                SCN_ASSERT(ScnFALSE) //missing implementation
                break;
        }
        ++c;
    }
    return r;
}

ScnBOOL ScnApiMetal_renderJob_buildEndAndEnqueue(void* data){
    ScnBOOL r = ScnFALSE;
    STScnApiMetalRenderJob* obj = (STScnApiMetalRenderJob*)data;
    STScnApiMetalRenderJobState* state = &obj->state;
    if(state->rndrEnc == nil || obj->cmdsBuff == nil || state->fb == NULL){
        return ScnFALSE;
    }
    //
    r = ScnTRUE;
    //finalize
    if(state->rndrEnc != nil){
        [state->rndrEnc endEncoding];
        SCN_PRINTF_VERB("endEncoding.\n");
    }
    if(obj->cmdsBuff != nil){
        if(state->fb != NULL && state->fb->mtkView != NULL){
            [obj->cmdsBuff presentDrawable:state->fb->mtkView.currentDrawable];
            SCN_PRINTF_VERB("presentDrawable.\n");
        }
        [obj->cmdsBuff commit];
        SCN_PRINTF_VERB("commit.\n");
    }
    return r;
}


// STScnApiMetalRenderJobState

void ScnApiMetalRenderJobState_init(STScnApiMetalRenderJobState* obj){
    ScnMemory_setZeroSt(*obj);
}

void ScnApiMetalRenderJobState_destroy(STScnApiMetalRenderJobState* obj){
    if(obj->rndrDesc != nil){ [obj->rndrDesc release]; obj->rndrDesc = nil; }
    if(obj->rndrEnc != nil){ [obj->rndrEnc release]; obj->rndrEnc = nil; }
}

void ScnApiMetalRenderJobState_reset(STScnApiMetalRenderJobState* obj){
    if(obj->rndrDesc != nil){ [obj->rndrDesc release]; obj->rndrDesc = nil; }
    if(obj->rndrEnc != nil){ [obj->rndrEnc release]; obj->rndrEnc = nil; }
    ScnMemory_setZeroSt(*obj);
}
