//
//  ScnApiMetal.m
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/api/ScnApiMetal.h"
#include "ixrender/gpu/ScnGpuDevice.h"
#include "ixrender/gpu/ScnGpuBuffer.h"
#include "ixrender/scene/ScnRenderCmd.h"
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import <TargetConditionals.h>  //for TARGET_OS_* macros

//STScnGpuDeviceApiItf

ScnGpuDeviceRef     ScnApiMetal_allocDevice(ScnContextRef ctx, const STScnGpuDeviceCfg* cfg);
void                ScnApiMetal_device_free(void* obj);
void*               ScnApiMetal_device_getApiDevice(void* obj);
ScnGpuBufferRef     ScnApiMetal_device_allocBuffer(void* obj, ScnMemElasticRef mem);
ScnGpuVertexbuffRef ScnApiMetal_device_allocVertexBuff(void* obj, const STScnGpuVertexbuffCfg* cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff);
ScnGpuFramebuffRef  ScnApiMetal_device_allocFramebuffFromOSView(void* obj, void* mtkView);
ScnBOOL             ScnApiMetal_device_render(void* obj, ScnGpuBufferRef fbPropsBuff, ScnGpuBufferRef mdlsPropsBuff, const STScnRenderCmd* const cmds, const ScnUI32 cmdsSz);
//buffer
void                ScnApiMetal_buffer_free(void* data);
ScnBOOL             ScnApiMetal_buffer_sync(void* data, const STScnGpuBufferCfg* cfg, ScnMemElasticRef mem, const STScnGpuBufferChanges* changes);
//vertexbuff
void                ScnApiMetal_vertexbuff_free(void* data);
ScnBOOL             ScnApiMetal_vertexbuff_sync(void* data, const STScnGpuVertexbuffCfg* cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff);
ScnBOOL             ScnApiMetal_vertexbuff_activate(void* data);
ScnBOOL             ScnApiMetal_vertexbuff_deactivate(void* data);
//frameBuffer (view)
void                ScnApiMetal_framebuff_view_free(void* data);
STScnSize2DU          ScnApiMetal_framebuff_view_getSize(void* data, STScnRectU* dstViewport);
ScnBOOL             ScnApiMetal_framebuff_view_syncSizeAndViewport(void* data, const STScnSize2DU size, const STScnRectU viewport);

ScnBOOL ScnApiMetal_getApiItf(STScnApiItf* dst){
    if(dst == NULL) return ScnFALSE;
    //
    memset(dst, 0, sizeof(*dst));
    //gobal
    dst->allocDevice        = ScnApiMetal_allocDevice;
    //device
    dst->dev.free           = ScnApiMetal_device_free;
    dst->dev.getApiDevice   = ScnApiMetal_device_getApiDevice;
    dst->dev.allocBuffer    = ScnApiMetal_device_allocBuffer;
    dst->dev.allocVertexBuff = ScnApiMetal_device_allocVertexBuff;
    dst->dev.allocFramebuffFromOSView = ScnApiMetal_device_allocFramebuffFromOSView;
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
                    memset(obj, 0, sizeof(*obj));
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
                        memset(obj, 0, sizeof(*obj));
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

ScnBOOL ScnApiMetal_buffer_sync(void* pObj, const STScnGpuBufferCfg* cfg, ScnMemElasticRef mem, const STScnGpuBufferChanges* changes){
    ScnBOOL r = ScnFALSE;
    STScnApiMetalBuffer* obj = (STScnApiMetalBuffer*)pObj;
    if(obj->buff == nil){
        printf("ERROR, ScnApiMetal_buffer_sync obj->buff is nil.\n");
    } else if(ScnMemElastic_isNull(mem)){
        printf("ERROR, ScnApiMetal_buffer_sync mem is NULL.\n");
    } else {
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
    if(rngsUse <= 0) return ScnTRUE;
    const STScnRangeU* rng = rngs;
    const STScnRangeU* rngAfterEnd = rngs + rngsUse;
    //
    const ScnUI32 buffLen = (ScnUI32)buff.length;
    ScnBYTE* buffPtr = (ScnBYTE*)buff.contents;
    STScnAbsPtr ptr = STScnAbsPtr_Zero;
    ScnUI32 continuousSz = 0, copySz = 0, bytesCopied = 0;
    //
    STScnRangeU curRng = { rng->start,  rng->size };
    ++rng;
    //
    while(rng <= rngAfterEnd){
        if( rng == rngAfterEnd || (curRng.start + curRng.size) != rng->start ){
            //copy current accumulated range
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
                memcpy(&buffPtr[curRng.start], ptr.ptr, copySz);
                bytesCopied += copySz;
                //
                SCN_ASSERT(curRng.size >= copySz)
                curRng.start += copySz;
                curRng.size -= copySz;
            }
            //end cycle
            if(rng == rngAfterEnd) break;
        }
        //merge range
        SCN_ASSERT((curRng.start + curRng.size) == rng->start)
        curRng.size += rng->size;
        //next
        ++rng;
    }
    SCN_ASSERT((curRng.start + curRng.size) == (rngs[rngsUse - 1].start + rngs[rngsUse - 1].size))
    printf("%2.f%% %u of %u bytes synced at buffer.\n", (float)bytesCopied * 100.f / (float)buffLen, bytesCopied, buffLen);
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

ScnGpuVertexbuffRef ScnApiMetal_device_allocVertexBuff(void* pObj, const STScnGpuVertexbuffCfg* cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff){
    ScnGpuVertexbuffRef r = ScnObjRef_Zero;
    STScnApiMetalDevice* dev = (STScnApiMetalDevice*)pObj;
    if(dev != NULL && dev->dev != NULL && cfg != NULL && !ScnGpuBuffer_isNull(vBuff)){ //idxBuff is optional
        //synced
        STScnApiMetalVertexBuff* obj = (STScnApiMetalVertexBuff*)ScnContext_malloc(dev->ctx, sizeof(STScnApiMetalVertexBuff), "STScnApiMetalVertexBuff");
        if(obj == NULL){
            printf("ScnContext_malloc(STScnApiMetalVertexBuff) failed.\n");
        } else {
            memset(obj, 0, sizeof(*obj));
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

ScnBOOL ScnApiMetal_vertexbuff_sync(void* pObj, const STScnGpuVertexbuffCfg* cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff){
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

//STScnApiMetalFramebuffView

typedef struct STScnApiMetalFramebuffView {
    ScnContextRef       ctx;
    MTKView*            mtkView;
    STScnSize2DU        size;
    STScnRectU          viewport;
    STScnApiItf         itf;
    //default shaders
    id<MTLRenderPipelineState> renderState; //shader and fragment for this framebuffer
} STScnApiMetalFramebuffView;

ScnGpuFramebuffRef ScnApiMetal_device_allocFramebuffFromOSView(void* pObj, void* pMtkView){
    ScnGpuFramebuffRef r = ScnObjRef_Zero;
    STScnApiMetalDevice* dev = (STScnApiMetalDevice*)pObj;
    MTKView* mtkView = (MTKView*)pMtkView;
    if(dev != NULL && dev->dev != NULL && mtkView != nil){
        STScnApiMetalFramebuffView* obj = NULL;
        id<MTLRenderPipelineState> renderState = nil;
        id<MTLFunction> vertexFunc = [dev->lib newFunctionWithName:@"vertexShader"];
        id<MTLFunction> fragmtFunc = [dev->lib newFunctionWithName:@"fragmentShader"];
        MTLRenderPipelineDescriptor* rndrPipeDesc = [[MTLRenderPipelineDescriptor alloc] init];
        if(vertexFunc == nil || fragmtFunc == nil || rndrPipeDesc == nil){
            printf("newFunctionWithName or [MTLRenderPipelineDescriptor alloc] failed.\n");
        } else {
            NSError *error;
            rndrPipeDesc.label = @"Ixtla-render default (fixed) render pipeline for 'STScnVertex2D' type.";
            rndrPipeDesc.vertexFunction = vertexFunc;
            rndrPipeDesc.fragmentFunction = fragmtFunc;
            rndrPipeDesc.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat;
            if(nil == (renderState = [dev->dev newRenderPipelineStateWithDescriptor:rndrPipeDesc error:&error])){
                printf("newRenderPipelineStateWithDescriptor failed: '%s'.\n", (error == nil ? "nil" : [[error description] UTF8String]));
            } else if(NULL == (obj = (STScnApiMetalFramebuffView*)ScnContext_malloc(dev->ctx, sizeof(STScnApiMetalFramebuffView), "STScnApiMetalFramebuffView"))){
                printf("ScnContext_malloc(STScnApiMetalFramebuffView) failed.\n");
            } else {
                CGSize viewSz = mtkView.drawableSize;
                memset(obj, 0, sizeof(*obj));
                ScnContext_set(&obj->ctx, dev->ctx);
                obj->itf            = dev->itf;
                obj->mtkView        = mtkView; [obj->mtkView retain];
                obj->size.width     = viewSz.width;
                obj->size.height    = viewSz.height;
                obj->viewport.x     = 0;
                obj->viewport.y     = 0;
                obj->viewport.width = obj->size.width;
                obj->viewport.height = obj->size.height;
                obj->renderState    = renderState; [obj->renderState retain];
                //
                ScnGpuFramebuffRef d = ScnGpuFramebuff_alloc(dev->ctx);
                if(!ScnGpuFramebuff_isNull(d)){
                    STScnGpuFramebuffApiItf itf;
                    memset(&itf, 0, sizeof(itf));
                    itf.free        = ScnApiMetal_framebuff_view_free;
                    itf.getSize     = ScnApiMetal_framebuff_view_getSize;
                    itf.syncSizeAndViewport = ScnApiMetal_framebuff_view_syncSizeAndViewport;
                    if(!ScnGpuFramebuff_prepare(d, &itf, obj)){
                        printf("ScnApiMetal_device_allocFramebuffFromOSView::ScnGpuFramebuff_prepare failed.\n");
                    } else {
                        ScnGpuFramebuff_set(&r, d);
                        obj = NULL; //consume
                    }
                    ScnGpuFramebuff_releaseAndNull(&d);
                }
            }
        }
        //release (if not consumed)
        if(obj != NULL){
            ScnApiMetal_framebuff_view_free(obj);
            obj = NULL;
        }
        if(renderState != nil){ [renderState release]; renderState = nil; }
        if(rndrPipeDesc != nil){ [rndrPipeDesc release]; rndrPipeDesc = nil; }
        if(vertexFunc != nil){ [vertexFunc release]; vertexFunc = nil; }
        if(fragmtFunc != nil){ [fragmtFunc release]; fragmtFunc = nil; }
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
                                if(fb == NULL || fb->mtkView == nil || fb->renderState == nil){
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
                                            MTLViewport viewPort;
                                            viewPort.originX = (double)fb->viewport.x;
                                            viewPort.originY = (double)fb->viewport.y;
                                            viewPort.width = (double)fb->viewport.width;
                                            viewPort.height = (double)fb->viewport.height;
                                            viewPort.znear = 0.0;
                                            viewPort.zfar = 1.0;
                                            [rndrEnc setViewport:viewPort];
                                            //printf("setViewport(%u, %u)-(+%u, +%u).\n", fb->viewport.x, fb->viewport.y, fb->viewport.width, fb->viewport.height);
                                            [rndrEnc setRenderPipelineState:fb->renderState];
                                            //printf("setRenderPipelineState.\n");
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
                        //models
                    case ENScnRenderCmd_SetTransformOffset: //sets the positions of the 'STScnGpuModelProps2D' to be applied for the drawing cmds
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
                            //ScnVertexbuff_get
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
                                        [rndrEnc setVertexBuffer:buff->buff offset:0 atIndex:2];
                                        //printf("setVertexBuffer(idx2, 0 offset).\n");
                                    }
                                }
                            }
                        }
                        break;
                        //case ENScnRenderCmd_SetTexture:     //activates the texture in a specific slot-index
                        //    break;
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
        if(obj->renderState != nil){
            [obj->renderState release];
            obj->renderState = nil;
        }
        if(obj->mtkView != nil){
            [obj->mtkView release];
            obj->mtkView = nil;
        }
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

STScnSize2DU ScnApiMetal_framebuff_view_getSize(void* pObj, STScnRectU* dstViewport){
    STScnApiMetalFramebuffView* obj = (STScnApiMetalFramebuffView*)pObj;
    if(dstViewport != NULL) *dstViewport = obj->viewport;
    return obj->size;
}

ScnBOOL ScnApiMetal_framebuff_view_syncSizeAndViewport(void* pObj, const STScnSize2DU size, const STScnRectU viewport){
    ScnBOOL r = ScnFALSE;
    STScnApiMetalFramebuffView* obj = (STScnApiMetalFramebuffView*)pObj;
    if(obj->mtkView != nil && size.width > 0 && size.height > 0){
        obj->size = size;
        obj->viewport = viewport;
        r = ScnTRUE;
    }
    return r;
}
