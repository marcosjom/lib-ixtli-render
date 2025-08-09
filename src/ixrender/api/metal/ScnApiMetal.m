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
ScnBOOL             ScnApiMetal_device_render(void* obj, ScnGpuBufferRef fbPropsBuff, ScnGpuBufferRef mdlsPropsBuff, const STScnRenderCmd* const cmds, const ScnUI32 cmdsSz);
//buffer
void                ScnApiMetal_buffer_free(void* data);
ScnBOOL             ScnApiMetal_buffer_sync(void* data, const STScnGpuBufferCfg* const cfg, ScnMemElasticRef mem, const STScnGpuBufferChanges* changes);
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
    dst->dev.render         = ScnApiMetal_device_render;
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
                printf("Metal, error, could not retrieve devices.\n");
            } else {
                ScnBOOL devIsExplicitMatch = ScnFALSE;
                printf("Metal, %d devices found:\n", (int)devs.count);
                int i; for(i = 0; i < devs.count; i++){
                    id<MTLDevice> d = devs[i];
                    printf("    ---------\n");
                    printf("    Dev#%d/%d: '%s'\n", (i + 1), (int)devs.count, [d.name UTF8String]);
                    //Identification
                    if(@available(iOS 18, macOS 14.0, *)){
                    printf("         Arch: '%s'\n", [d.architecture.name UTF8String]);
                    }
                    printf("          Loc: '%s' (num: %d)\n", d.location == MTLDeviceLocationBuiltIn ? "BuiltIn" : d.location == MTLDeviceLocationSlot ? "Slot" : d.location == MTLDeviceLocationExternal ? "External" : d.location == MTLDeviceLocationUnspecified ? "Unspecified" :"Unknown", (int)d.locationNumber);
                    printf("       LowPwr: %s\n", d.isLowPower ? "yes" : "no");
                    printf("    Removable: %s\n", d.isRemovable ? "yes" : "no");
                    printf("     Headless: %s\n", d.isHeadless ? "yes" : "no");
                    printf("         Peer: grpId(%llu) idx(%d) count(%d)\n", (ScnUI64)d.peerGroupID, d.peerIndex, d.peerCount);
                    //GPU's Device Memory
                    printf("     CurAlloc: %.2f %s\n", (double)d.currentAllocatedSize / (d.currentAllocatedSize >= (1024 * 1024 * 1024) ? (double)(1024 * 1024 * 1024) : d.currentAllocatedSize >= (1024 * 1024) ? (double)(1024 * 1024) : d.currentAllocatedSize >= (1024) ? (double)(1024) : 1.0), (d.currentAllocatedSize >= (1024 * 1024 * 1024) ? "GBs" : d.currentAllocatedSize >= (1024 * 1024) ? "MBs" : d.currentAllocatedSize >= (1024) ? "KBs" : "bytes"));
                    printf("     MaxAlloc: %.2f %s (recommended)\n", (double)d.recommendedMaxWorkingSetSize / (d.recommendedMaxWorkingSetSize >= (1024 * 1024 * 1024) ? (double)(1024 * 1024 * 1024) : d.recommendedMaxWorkingSetSize >= (1024 * 1024) ? (double)(1024 * 1024) : d.recommendedMaxWorkingSetSize >= (1024) ? (double)(1024) : 1.0), (d.recommendedMaxWorkingSetSize >= (1024 * 1024 * 1024) ? "GBs" : d.recommendedMaxWorkingSetSize >= (1024 * 1024) ? "MBs" : d.recommendedMaxWorkingSetSize >= (1024) ? "KBs" : "bytes"));
                    printf("      MaxRate: %.2f %s/s\n", (double)d.maxTransferRate / (d.maxTransferRate >= (1024 * 1024 * 1024) ? (double)(1024 * 1024 * 1024) : d.maxTransferRate >= (1024 * 1024) ? (double)(1024 * 1024) : d.maxTransferRate >= (1024) ? (double)(1024) : 1.0), (d.maxTransferRate >= (1024 * 1024 * 1024) ? "GBs" : d.maxTransferRate >= (1024 * 1024) ? "MBs" : d.maxTransferRate >= (1024) ? "KBs" : "bytes"));
                    printf("   UnifiedMem: %s\n", d.hasUnifiedMemory ? "yes" : "no");
                    //Compute Support
                    printf(" ThreadGrpMem: %.2f %s\n", (double)d.maxThreadgroupMemoryLength / (d.maxThreadgroupMemoryLength >= (1024 * 1024 * 1024) ? (double)(1024 * 1024 * 1024) : d.maxThreadgroupMemoryLength >= (1024 * 1024) ? (double)(1024 * 1024) : d.maxThreadgroupMemoryLength >= (1024) ? (double)(1024) : 1.0), (d.maxThreadgroupMemoryLength >= (1024 * 1024 * 1024) ? "GBs" : d.maxThreadgroupMemoryLength >= (1024 * 1024) ? "MBs" : d.maxThreadgroupMemoryLength >= (1024) ? "KBs" : "bytes"));
                    printf("  ThrdsPerGrp: (%d, %d, %d)\n", (int)d.maxThreadsPerThreadgroup.width, (int)d.maxThreadsPerThreadgroup.height, (int)d.maxThreadsPerThreadgroup.depth);
                    //Functions Pointer Support
                    printf("     FuncPtrs: %s (compute kernel functions)\n", d.supportsFunctionPointers ? "yes" : "no");
                    if(@available(iOS 18, macOS 12.0, *)){
                    printf("    FPtrsRndr: %s\n", d.supportsFunctionPointersFromRender ? "yes" : "no");
                    }
                    //Texture and sampler support
                    printf("   32bFltFilt: %s\n", d.supports32BitFloatFiltering ? "yes" : "no");
                    printf("   BCTextComp: %s\n", d.supportsBCTextureCompression ? "yes" : "no");
                    printf("    Depth24-8: %s\n", d.isDepth24Stencil8PixelFormatSupported ? "yes" : "no");
                    printf("  TexLODQuery: %s\n", d.supportsQueryTextureLOD ? "yes" : "no");
                    printf("        RWTex: %s\n", d.readWriteTextureSupport ? "yes" : "no");
                    //Render support
                    printf("   RayTracing: %s\n", d.supportsRaytracing ? "yes" : "no");
                    if(@available(iOS 18, macOS 12.0, *)){
                    printf("  RayTracRndr: %s\n", d.supportsRaytracingFromRender ? "yes" : "no");
                    }
                    printf("  PrimMotBlur: %s\n", d.supportsPrimitiveMotionBlur ? "yes" : "no");
                    printf("      32bMSAA: %s\n", d.supports32BitMSAA ? "yes" : "no");
                    printf("  PullModeInt: %s\n", d.supportsPullModelInterpolation ? "yes" : "no");
                    printf("ShadBaryCoord: %s\n", d.supportsShaderBarycentricCoordinates ? "yes" : "no");
                    printf("  ProgSmplPos: %s\n", d.areProgrammableSamplePositionsSupported ? "yes" : "no");
                    printf("  RstrOrdGrps: %s\n", d.areRasterOrderGroupsSupported ? "yes" : "no");
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
                    printf("    ---------\n");
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
            printf("Metal, error, could select a device.\n");
        } else {
            printf("Selected device: '%s'\n", [dev.name UTF8String]);
            id<MTLLibrary> defLib = [dev newDefaultLibrary];
            if (defLib == nil){
                printf("Metal, error, newDefaultLibrary failed.\n");
            } else if(nil == (cmdQueue = [dev newCommandQueue])){
                printf("Metal, error, newCommandQueue failed.\n");
            } else {
                STScnApiMetalDevice* obj = (STScnApiMetalDevice*)ScnContext_malloc(ctx, sizeof(STScnApiMetalDevice), "STScnApiMetalDevice");
                if(obj == NULL){
                    printf("ScnContext_malloc(STScnApiMetalDevice) failed.\n");
                } else {
                    //init STScnApiMetalDevice
                    ScnMemory_setZeroSt(*obj);
                    ScnContext_set(&obj->ctx, ctx);
                    obj->dev    = dev; [dev retain];
                    obj->lib    = defLib; [defLib retain];
                    obj->cmdQueue = cmdQueue; [cmdQueue retain];
                    //
                    if(!ScnApiMetal_getApiItf(&obj->itf)){
                        printf("ScnApiMetal_allocDevice::ScnApiMetal_getApiItf failed.\n");
                    } else {
                        ScnGpuDeviceRef d = ScnGpuDevice_alloc(ctx);
                        if(!ScnGpuDevice_isNull(d)){
                            if(!ScnGpuDevice_prepare(d, &obj->itf.dev, obj)){
                                printf("ScnApiMetal_allocDevice::ScnGpuDevice_prepare failed.\n");
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
            /*if(defLib != nil){
                [defLib release];
                defLib = nil;
            }*/
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
            printf("[MTLSamplerDescriptor new] failed.\n");
        } else {
            const MTLSamplerAddressMode addressMode = (cfg->address == ENScnGpusamplerAddress_Clamp ? MTLSamplerAddressModeClampToEdge : MTLSamplerAddressModeRepeat);
            desc.minFilter = (cfg->magFilter == ENScnGpuSamplerFilter_Linear ? MTLSamplerMinMagFilterLinear : MTLSamplerMinMagFilterNearest);
            desc.magFilter = (cfg->minFilter == ENScnGpuSamplerFilter_Linear ? MTLSamplerMinMagFilterLinear : MTLSamplerMinMagFilterNearest);
            desc.sAddressMode = desc.tAddressMode = desc.rAddressMode = addressMode;
            id<MTLSamplerState> sampler = [dev->dev newSamplerStateWithDescriptor:desc];
            if(sampler == nil){
                printf("[dev newSamplerStateWithDescriptor] failed.\n");
            } else {
                STScnApiMetalSampler* obj = (STScnApiMetalSampler*)ScnContext_malloc(dev->ctx, sizeof(STScnApiMetalSampler), "STScnApiMetalSampler");
                if(obj == NULL){
                    printf("ScnContext_malloc(STScnApiMetalSampler) failed.\n");
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
                            printf("ScnApiMetal_device_allocSampler::ScnGpuSampler_prepare failed.\n");
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
        r.isTexFmtInfoRequired = ScnTRUE;   //fragment shader requires the textures format info to produce correct color output
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
            printf("ERROR, allocating zero-sz gpu buffer is not allowed.\n");
        } else {
            id<MTLBuffer> buff = [dev->dev newBufferWithLength:cpuBuffSz options:MTLResourceStorageModeShared];
            if(buff != nil){
                const STScnRangeU rngAll = ScnMemElastic_getUsedAddressesRng(mem);
                if(!ScnApiMetal_buffer_syncRanges_(buff, mem, &rngAll, 1)){
                    printf("ERROR, ScnApiMetal_buffer_syncRanges_ failed.\n");
                } else {
                    //synced
                    STScnApiMetalBuffer* obj = (STScnApiMetalBuffer*)ScnContext_malloc(dev->ctx, sizeof(STScnApiMetalBuffer), "STScnApiMetalBuffer");
                    if(obj == NULL){
                        printf("ScnContext_malloc(STScnApiMetalBuffer) failed.\n");
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
                                printf("ScnApiMetal_allocDevice::ScnGpuBuffer_prepare failed.\n");
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

ScnBOOL ScnApiMetal_buffer_sync(void* pObj, const STScnGpuBufferCfg* const cfg, ScnMemElasticRef mem, const STScnGpuBufferChanges* changes){
    ScnBOOL r = ScnFALSE;
    STScnApiMetalBuffer* obj = (STScnApiMetalBuffer*)pObj;
    if(obj->buff == nil || ScnMemElastic_isNull(mem)){
        //missing params
        return r;
    }
    //sync
    {
        ScnBOOL buffIsNew = ScnFALSE;
        ScnUI32 buffLen = (ScnUI32)obj->buff.length;
        const ScnUI32 cpuBuffSz = ScnMemElastic_getAddressableSize(mem);
        //resize (if necesary)
        if(cpuBuffSz != buffLen){
            //recreate buffer
            id<MTLBuffer> buff = [obj->dev newBufferWithLength:cpuBuffSz options:MTLResourceStorageModeShared];
            if(buff != nil){
                printf("ScnApiMetal_buffer_sync::gpu-buff resized from %u to %u bytes.\n", buffLen, cpuBuffSz);
                buffLen = cpuBuffSz;
                [obj->buff release];
                obj->buff = buff;
                buffIsNew = ScnTRUE;
            }
        }
        //sync
        if(buffLen < cpuBuffSz){
            printf("ERROR, ScnApiMetal_buffer_sync::gpuBuff is smaller than cpu-buff.\n");
        } else if(buffIsNew || changes->all){
            //sync all
            const STScnRangeU rngAll = ScnMemElastic_getUsedAddressesRng(mem);
            if(!ScnApiMetal_buffer_syncRanges_(obj->buff, mem, &rngAll, 1)){
                printf("ERROR, ScnApiMetal_buffer_sync::ScnApiMetal_buffer_syncRanges_ failed.\n");
            } else {
                r = ScnTRUE;
            }
        } else {
            //sync ranges only
            if(!ScnApiMetal_buffer_syncRanges_(obj->buff, mem, changes->rngs, changes->rngsUse)){
                printf("ERROR, ScnApiMetal_buffer_sync::ScnApiMetal_buffer_syncRanges_ failed.\n");
            } else {
                r = ScnTRUE;
            }
        }
    }
    return r;
}

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
                printf("ERROR, gpu-buffer is smaller than cpu-buffer.\n");
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
    //printf("%2.f%% %u of %u bytes synced at buffer.\n", (float)bytesCopied * 100.f / (float)buffLen, bytesCopied, buffLen);
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
        STScnApiMetalVertexBuff* obj = (STScnApiMetalVertexBuff*)ScnContext_malloc(dev->ctx, sizeof(STScnApiMetalVertexBuff), "STScnApiMetalVertexBuff");
        if(obj == NULL){
            printf("ScnContext_malloc(STScnApiMetalVertexBuff) failed.\n");
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
                    printf("ScnApiMetal_device_allocVertexBuff::ScnGpuVertexbuff_prepare failed.\n");
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
            printf("ERROR, unsupported texture color format(%d).\n", cfg->color);
        } else if(cfg->width <= 0 && cfg->height <= 0){
            printf("ERROR, invalid texture size(%d, %d).\n", cfg->width, cfg->height);
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
                printf("ERROR, newTextureWithDescriptor failed.\n");
            } else if(srcProps != NULL && (srcProps->size.width != cfg->width || srcProps->size.height != cfg->height || srcProps->color != cfg->color || srcProps->bytesPerLine <= 0 || srcData == NULL)){
                printf("ERROR, texture and source props missmatch.\n");
            } else if(NULL == (obj = (STScnApiMetalTexture*)ScnContext_malloc(dev->ctx, sizeof(STScnApiMetalTexture), "STScnApiMetalTexture"))){
                printf("ScnContext_malloc(STScnApiMetalTexture) failed.\n");
            } else {
                ScnMemory_setZeroSt(*obj);
                ScnContext_set(&obj->ctx, dev->ctx);
                obj->itf            = dev->itf;
                obj->tex            = tex; [obj->tex retain];
                obj->cfg            = *cfg;
                obj->sampler        = ScnApiMetal_device_allocSampler(dev, &cfg->sampler);
                if(ScnGpuSampler_isNull(obj->sampler)){
                    printf("ScnApiMetal_device_allocTexture::ScnApiMetal_device_allocSampler failed.\n");
                } else {
                    ScnGpuTextureRef d = ScnGpuTexture_alloc(dev->ctx);
                    if(!ScnGpuTexture_isNull(d)){
                        STScnGpuTextureApiItf itf;
                        ScnMemory_setZeroSt(itf);
                        itf.free        = ScnApiMetal_texture_free;
                        itf.sync        = ScnApiMetal_texture_sync;
                        if(!ScnGpuTexture_prepare(d, &itf, obj)){
                            printf("ScnApiMetal_device_allocTexture::ScnGpuTexture_prepare failed.\n");
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
        printf("%2.f%% %u of %u lines synced at texture (%.1f Kpixs, %.1f KBs).\n", (float)lnsUpdated * 100.f / (float)obj->cfg.height, lnsUpdated, obj->cfg.height, (ScnFLOAT)pxUpdated / 1024.f, (ScnFLOAT)lnsUpdated * (ScnFLOAT)srcProps->bytesPerLine / 1024.f);
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
    ENScnVertexType         curVertexType;
    STScnApiMetalRenderStates renderStates;
} STScnApiMetalFramebuffView;

ScnGpuFramebuffRef ScnApiMetal_device_allocFramebuffFromOSView(void* pObj, void* pMtkView){
    ScnGpuFramebuffRef r = ScnObjRef_Zero;
    STScnApiMetalDevice* dev = (STScnApiMetalDevice*)pObj;
    MTKView* mtkView = (MTKView*)pMtkView;
    if(dev != NULL && dev->dev != NULL && mtkView != nil){
        STScnApiMetalFramebuffView* obj = NULL;
        STScnApiMetalRenderStates renderStates;
        STScnApiMetalRenderStates_init(&renderStates);
        if(!STScnApiMetalRenderStates_load(&renderStates, dev, mtkView.colorPixelFormat)){
            printf("STScnApiMetalRenderStates_load failed.\n");
        } else if(NULL == (obj = (STScnApiMetalFramebuffView*)ScnContext_malloc(dev->ctx, sizeof(STScnApiMetalFramebuffView), "STScnApiMetalFramebuffView"))){
            printf("ScnContext_malloc(STScnApiMetalFramebuffView) failed.\n");
            STScnApiMetalRenderStates_destroy(&renderStates);
        } else {
            CGSize viewSz = mtkView.drawableSize;
            ScnMemory_setZeroSt(*obj);
            ScnContext_set(&obj->ctx, dev->ctx);
            obj->itf            = dev->itf;
            obj->mtkView        = mtkView; [obj->mtkView retain];
            obj->renderStates   = renderStates;
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
                    printf("ScnApiMetal_device_allocFramebuffFromOSView::ScnGpuFramebuff_prepare failed.\n");
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

ScnBOOL ScnApiMetal_device_render(void* pObj, ScnGpuBufferRef pFbPropsBuff, ScnGpuBufferRef pMdlsPropsBuff, const STScnRenderCmd* const cmds, const ScnUI32 cmdsSz){
    ScnBOOL r = ScnTRUE;
    STScnApiMetalDevice* dev = (STScnApiMetalDevice*)pObj;
    if(dev != NULL && dev->dev != nil && dev->cmdQueue != nil && !ScnGpuBuffer_isNull(pFbPropsBuff) && !ScnGpuBuffer_isNull(pMdlsPropsBuff)){
        STScnApiMetalBuffer* fbPropsBuff = (STScnApiMetalBuffer*)ScnGpuBuffer_getApiItfParam(pFbPropsBuff);
        STScnApiMetalBuffer* mdlsPropsBuff = (STScnApiMetalBuffer*)ScnGpuBuffer_getApiItfParam(pMdlsPropsBuff);
        if(fbPropsBuff == NULL || fbPropsBuff->buff == nil){
            //
        } else if(mdlsPropsBuff == NULL || mdlsPropsBuff->buff == nil){
            //
        } else {
            STScnApiMetalFramebuffView* fb = NULL;
            MTLRenderPassDescriptor* renderPassDescriptor = nil;
            id<MTLCommandBuffer> commandBuffer = nil;
            id<MTLRenderCommandEncoder> rndrEnc = nil;
            const STScnRenderCmd* c = cmds;
            const STScnRenderCmd* cAfterEnd = cmds + cmdsSz;
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
                                printf("ERROR, ENScnRenderCmd_ActivateFramebuff::ScnGpuFramebuff_isNull.\n");
                                r = ScnFALSE;
                            } else {
                                fb = (STScnApiMetalFramebuffView*)ScnGpuFramebuff_getApiItfParam(gpuFb);
                                if(fb == NULL || fb->mtkView == nil || fb->renderStates.states[fb->curVertexType] == nil){
                                    printf("ERROR, ENScnRenderCmd_ActivateFramebuff::fb == NULL || fb->mtkView == nil || fb->renderState == nil.\n");
                                    r = ScnFALSE;
                                } else {
                                    renderPassDescriptor = fb->mtkView.currentRenderPassDescriptor;
                                    //printf("currentRenderPassDescriptor.\n");
                                    commandBuffer = [dev->cmdQueue commandBuffer];
                                    //printf("commandBuffer.\n");
                                    if(renderPassDescriptor == nil || commandBuffer == nil){
                                        printf("ERROR, ENScnRenderCmd_ActivateFramebuff::renderPassDescriptor == nil || commandBuffer == nil.\n");
                                        r = ScnFALSE;
                                    } else {
                                        commandBuffer.label = @"Ixtli-cmd-buff";
                                        rndrEnc = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
                                        if(rndrEnc == nil){
                                            printf("ERROR, ENScnRenderCmd_ActivateFramebuff::renderCommandEncoderWithDescriptor failed.\n");
                                            r = ScnFALSE;
                                        } else {
                                            //printf("renderCommandEncoderWithDescriptor.\n");
                                            rndrEnc.label = @"ixtli-render-cmd-enc";
                                            //
                                            [rndrEnc setRenderPipelineState:fb->renderStates.states[fb->curVertexType]];
                                            //printf("setRenderPipelineState.\n");
                                            //apply viewport
                                            {
                                                MTLViewport viewPort;
                                                viewPort.originX = (double)c->activateFramebuff.viewport.x;
                                                viewPort.originY = (double)c->activateFramebuff.viewport.y;
                                                viewPort.width = (double)c->activateFramebuff.viewport.width;
                                                viewPort.height = (double)c->activateFramebuff.viewport.height;
                                                viewPort.znear = 0.0;
                                                viewPort.zfar = 1.0;
                                                [rndrEnc setViewport:viewPort];
                                                //printf("setViewport(%u, %u)-(+%u, +%u).\n", fb->viewport.x, fb->viewport.y, fb->viewport.width, fb->viewport.height);
                                            }
                                            //fb props
                                            [rndrEnc setVertexBuffer:fbPropsBuff->buff
                                                                    offset:c->activateFramebuff.offset
                                                                   atIndex:0];
                                            //printf("setVertexBuffer(idx0, %u offset).\n", c->activateFramebuff.offset);
                                            //mdl props
                                            [rndrEnc setVertexBuffer:mdlsPropsBuff->buff
                                                                    offset:0
                                                                   atIndex:1];
                                            //printf("setVertexBuffer(idx1, 0 offset).\n");
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    case ENScnRenderCmd_SetFramebuffProps:
                        if(rndrEnc != nil){
                            //apply viewport
                            {
                                MTLViewport viewPort;
                                viewPort.originX = (double)c->setFramebuffProps.viewport.x;
                                viewPort.originY = (double)c->setFramebuffProps.viewport.y;
                                viewPort.width = (double)c->setFramebuffProps.viewport.width;
                                viewPort.height = (double)c->setFramebuffProps.viewport.height;
                                viewPort.znear = 0.0;
                                viewPort.zfar = 1.0;
                                [rndrEnc setViewport:viewPort];
                                //printf("setViewport(%u, %u)-(+%u, +%u).\n", fb->viewport.x, fb->viewport.y, fb->viewport.width, fb->viewport.height);
                            }
                            //fb props
                            [rndrEnc setVertexBufferOffset:c->setFramebuffProps.offset
                                                   atIndex:0];
                        }
                        break;
                        //models
                    case ENScnRenderCmd_SetTransformOffset: //sets the positions of the 'STScnGpuModelProps2d' to be applied for the drawing cmds
                        if(rndrEnc == nil){
                            printf("ERROR, ENScnRenderCmd_SetTransformOffset::rndrEnc is nil.\n");
                            r = ScnFALSE;
                        } else {
                            [rndrEnc setVertexBufferOffset:c->setTransformOffset.offset atIndex:1];
                            //printf("setVertexBufferOffset(idx1, %u offset).\n", c->setTransformOffset.offset);
                        }
                        break;
                    case ENScnRenderCmd_SetVertexBuff:  //activates the vertex buffer
                        if(ScnVertexbuff_isNull(c->setVertexBuff.ref)){
                            printf("ERROR, ENScnRenderCmd_SetVertexBuff::ScnVertexbuff_isNull.\n");
                            [rndrEnc setVertexBuffer:nil offset:0 atIndex:2];
                            //printf("setVertexBuffer(idx2, nil).\n");
                        } else {
                            ScnGpuVertexbuffRef vbuffRef = ScnVertexbuff_getCurrentRenderSlot(c->setVertexBuff.ref);
                            if(ScnGpuVertexbuff_isNull(vbuffRef)){
                                printf("ERROR, ENScnRenderCmd_SetVertexBuff::ScnGpuVertexbuff_isNull.\n");
                                r = ScnFALSE;
                            } else {
                                STScnApiMetalVertexBuff* vbuff = (STScnApiMetalVertexBuff*)ScnGpuVertexBuff_getApiItfParam(vbuffRef);
                                if(vbuff == NULL || ScnGpuBuffer_isNull(vbuff->vBuff)){
                                    printf("ERROR, ENScnRenderCmd_SetVertexBuff::ScnGpuBuffer_isNull.\n");
                                    r = ScnFALSE;
                                } else {
                                    STScnApiMetalBuffer* buff = (STScnApiMetalBuffer*)ScnGpuBuffer_getApiItfParam(vbuff->vBuff);
                                    if(buff == NULL){
                                        printf("ERROR, ENScnRenderCmd_SetVertexBuff::uff == NULL.\n");
                                        r = ScnFALSE;
                                    } else {
                                        const ENScnVertexType vertexType = STScnGpuVertexbuffCfg_2_ENScnVertexType(&vbuff->cfg);
                                        if(fb->renderStates.states[fb->curVertexType] == nil){
                                            printf("ERROR, ENScnRenderCmd_SetVertexBuff::fb->renderStates.states[fb->curVertexType] == nil.\n");
                                            r = ScnFALSE;
                                        } else {
                                            fb->curVertexType = vertexType;
                                            [rndrEnc setRenderPipelineState:fb->renderStates.states[fb->curVertexType]];
                                            [rndrEnc setVertexBuffer:buff->buff offset:0 atIndex:2];
                                            //printf("setVertexBuffer(idx2, 0 offset).\n");
                                        }
                                    }
                                }
                            }
                        }
                        break;
                   case ENScnRenderCmd_SetTexture:     //activates the texture in a specific slot-index
                        if(ScnTexture_isNull(c->setTexture.ref)){
                            [rndrEnc setFragmentTexture:nil atIndex:c->setTexture.index];
                            [rndrEnc setFragmentSamplerState:nil atIndex:c->setTexture.index];
                        } else {
                            ScnGpuTextureRef texRef = ScnTexture_getCurrentRenderSlot(c->setTexture.ref);
                            if(ScnGpuTexture_isNull(texRef)){
                                printf("ERROR, ENScnRenderCmd_SetVertexBuff::ScnGpuTexture_isNull.\n");
                                r = ScnFALSE;
                            } else {
                                STScnApiMetalTexture* tex = (STScnApiMetalTexture*)ScnGpuTexture_getApiItfParam(texRef);
                                if(tex == NULL || tex->tex == nil || ScnGpuSampler_isNull(tex->sampler)){
                                    printf("ERROR, ENScnRenderCmd_SetVertexBuff::tex->tex is NULL.\n");
                                    r = ScnFALSE;
                                } else {
                                    STScnApiMetalSampler* smplr = (STScnApiMetalSampler*)ScnGpuSampler_getApiItfParam(tex->sampler);
                                    if(smplr->smplr == nil){
                                        printf("ERROR, ENScnRenderCmd_SetVertexBuff::smplr->smplr is NULL.\n");
                                        r = ScnFALSE;
                                    } else {
                                        [rndrEnc setFragmentTexture:tex->tex atIndex:c->setTexture.index];
                                        [rndrEnc setFragmentSamplerState:smplr->smplr atIndex:c->setTexture.index];
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
                                if(rndrEnc == NULL){
                                    //rintf("ERROR, ENScnRenderShape_Texture::rndrEnc == NULL.\n");
                                    r = ScnFALSE;
                                } else {
                                    [rndrEnc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:c->drawVerts.iFirst vertexCount:c->drawVerts.count];
                                    //printf("drawPrimitives(MTLPrimitiveTypeTriangleStrip: %u, +%u).\n", c->drawVerts.iFirst, c->drawVerts.count);
                                }
                                break;
                            case ENScnRenderShape_TriangStrip: //triangles-strip, most common shape
                                if(rndrEnc == NULL){
                                    printf("ERROR, ENScnRenderShape_TriangStrip::rndrEnc == NULL.\n");
                                    r = ScnFALSE;
                                } else {
                                    [rndrEnc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:c->drawVerts.iFirst vertexCount:c->drawVerts.count];
                                    //printf("drawPrimitives(MTLPrimitiveTypeTriangleStrip: %u, +%u).\n", c->drawVerts.iFirst, c->drawVerts.count);
                                }
                                break;
                            //case ENScnRenderShape_TriangFan:   //triangles-fan
                            //    break;
                                //
                            case ENScnRenderShape_LineStrip:   //lines-strip
                                if(rndrEnc == NULL){
                                    printf("ERROR, ENScnRenderShape_LineStrip::rndrEnc == NULL.\n");
                                    r = ScnFALSE;
                                } else {
                                    [rndrEnc drawPrimitives:MTLPrimitiveTypeLineStrip vertexStart:c->drawVerts.iFirst vertexCount:c->drawVerts.count];
                                    //printf("drawPrimitives(MTLPrimitiveTypeLineStrip: %u, +%u).\n", c->drawVerts.iFirst, c->drawVerts.count);
                                }
                                break;
                            //case ENScnRenderShape_LineLoop:    //lines-loop
                            //    break;
                            case ENScnRenderShape_Lines:       //lines
                                if(rndrEnc == NULL){
                                    printf("ERROR, ENScnRenderShape_Lines::rndrEnc == NULL.\n");
                                    r = ScnFALSE;
                                } else {
                                    [rndrEnc drawPrimitives:MTLPrimitiveTypeLine vertexStart:c->drawVerts.iFirst vertexCount:c->drawVerts.count];
                                    //printf("drawPrimitives(MTLPrimitiveTypeLine: %u, +%u).\n", c->drawVerts.iFirst, c->drawVerts.count);
                                }
                                break;
                            case ENScnRenderShape_Points:      //points
                                if(rndrEnc == NULL){
                                    printf("ERROR, ENScnRenderShape_Points::rndrEnc == NULL.\n");
                                    r = ScnFALSE;
                                } else {
                                    [rndrEnc drawPrimitives:MTLPrimitiveTypePoint vertexStart:c->drawVerts.iFirst vertexCount:c->drawVerts.count];
                                    //printf("drawPrimitives(MTLPrimitiveTypePoint: %u, +%u).\n", c->drawVerts.iFirst, c->drawVerts.count);
                                }
                                break;
                            default:
                                SCN_ASSERT(ScnFALSE); //missing implementation
                                break;
                        }
                        break;
                    case ENScnRenderCmd_DrawIndexes:    //draws something using the vertices indexes
                        break;
                    default:
                        SCN_ASSERT(ScnFALSE) //missing implementation
                        break;
                }
                ++c;
            }
            //finalize
            if(rndrEnc != nil){
                [rndrEnc endEncoding];
                //printf("endEncoding.\n");
            }
            if(commandBuffer != nil){
                if(fb != NULL && fb->mtkView != NULL){
                    [commandBuffer presentDrawable:fb->mtkView.currentDrawable];
                    //printf("presentDrawable.\n");
                }
                [commandBuffer commit];
                //printf("commit.\n");
            }
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
        STScnApiMetalRenderStates_destroy(&obj->renderStates);
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
        ScnSI32 i; for(i = 0; i < (sizeof(obj->states) / sizeof(obj->states[0])); ++i){
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
                printf("STScnApiMetalRenderStates_load unimplemented function name.\n");
                r = ScnFALSE;
                break;
            }
            if(fragmtFuncName == NULL){
                printf("STScnApiMetalRenderStates_load unimplemented function name.\n");
                r = ScnFALSE;
                break;
            }
            //
            if(vertexFunc != nil){ [vertexFunc release]; vertexFunc = nil; }
            if(fragmtFunc != nil){ [fragmtFunc release]; fragmtFunc = nil; }
            //
            vertexFunc = [dev->lib newFunctionWithName:[NSString stringWithUTF8String:vertexFuncName]];
            if(vertexFunc == nil){
                printf("STScnApiMetalRenderStates_load newFunctionWithName('%s') failed.\n", vertexFuncName);
                r = ScnFALSE;
                break;
            }
            fragmtFunc = [dev->lib newFunctionWithName:[NSString stringWithUTF8String:fragmtFuncName]];
            if(fragmtFunc == nil){
                printf("STScnApiMetalRenderStates_load newFunctionWithName('%s') failed.\n", fragmtFuncName);
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
                    printf("STScnApiMetalRenderStates_load::newRenderPipelineStateWithDescriptor failed: '%s'.\n", (error == nil ? "nil" : [[error description] UTF8String]));
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
