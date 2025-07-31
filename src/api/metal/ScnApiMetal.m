//
//  ScnApiMetal.m
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/api/ScnApiMetal.h"
#include "ixrender/gpu/ScnGpuDevice.h"
#include "ixrender/gpu/ScnGpuBuffer.h"
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>

//STScnGpuDeviceApiItf

ScnGpuDeviceRef     ScnApiMetal_allocDevice(ScnContextRef ctx, const STScnGpuDeviceCfg* cfg);
void                ScnApiMetal_device_free(void* obj);
ScnGpuBufferRef     ScnApiMetal_device_allocBuffer(void* obj, ScnMemElasticRef mem);
ScnGpuVertexbuffRef ScnApiMetal_device_allocVertexBuff(void* obj, const STScnGpuVertexbuffCfg* cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff);
ScnGpuFramebuffRef  ScnApiMetal_device_allocFramebuffFromOSView(void* obj, void* mtkView);
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
STScnSizeU          ScnApiMetal_framebuff_view_getSize(void* data, STScnRectU* dstViewport);
ScnBOOL             ScnApiMetal_framebuff_view_syncSizeAndViewport(void* data, const STScnSizeU size, const STScnRectU viewport);

ScnBOOL ScnApiMetal_getApiItf(STScnApiItf* dst){
    if(dst == NULL) return ScnFALSE;
    //
    memset(dst, 0, sizeof(*dst));
    //gobal
    dst->allocDevice        = ScnApiMetal_allocDevice;
    //device
    dst->dev.free           = ScnApiMetal_device_free;
    dst->dev.allocBuffer    = ScnApiMetal_device_allocBuffer;
    dst->dev.allocVertexBuff = ScnApiMetal_device_allocVertexBuff;
    dst->dev.allocFramebuffFromOSView = ScnApiMetal_device_allocFramebuffFromOSView;
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

typedef struct STScnApiMetalDevice_ {
    ScnContextRef   ctx;
    STScnApiItf     itf;
    id<MTLDevice>   dev;
    id<MTLLibrary>  lib;
} STScnApiMetalDevice;

ScnGpuDeviceRef ScnApiMetal_allocDevice(ScnContextRef ctx, const STScnGpuDeviceCfg* cfg){
    ScnGpuDeviceRef r = ScnObjRef_Zero;
    if(!ScnContext_isNull(ctx)){
        NSArray<id<MTLDevice>> *devs = MTLCopyAllDevices();
        if(devs == nil){
            printf("Metal, error, could not retrieve devices.\n");
        } else {
            printf("Metal, %d devices found:\n", (int)devs.count);
            id<MTLDevice> dev = nil; ScnBOOL devIsExplicitMatch = ScnFALSE;
            int i; for(i = 0; i < devs.count; i++){
                id<MTLDevice> d = devs[i];
                printf("    ---------\n");
                printf("    Dev#%d/%d: '%s'\n", (i + 1), (int)devs.count, [d.name UTF8String]);
                //Identification
                printf("         Arch: '%s'\n", [d.architecture.name UTF8String]);
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
                printf("    FPtrsRndr: %s\n", d.supportsFunctionPointersFromRender ? "yes" : "no");
                //Texture and sampler support
                printf("   32bFltFilt: %s\n", d.supports32BitFloatFiltering ? "yes" : "no");
                printf("   BCTextComp: %s\n", d.supportsBCTextureCompression ? "yes" : "no");
                printf("    Depth24-8: %s\n", d.isDepth24Stencil8PixelFormatSupported ? "yes" : "no");
                printf("  TexLODQuery: %s\n", d.supportsQueryTextureLOD ? "yes" : "no");
                printf("        RWTex: %s\n", d.readWriteTextureSupport ? "yes" : "no");
                //Render support
                printf("   RayTracing: %s\n", d.supportsRaytracing ? "yes" : "no");
                printf("  RayTracRndr: %s\n", d.supportsRaytracingFromRender ? "yes" : "no");
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
            if(dev == nil){
                printf("Metal, error, could select a device.\n");
            } else {
                printf("Selected device: '%s'\n", [dev.name UTF8String]);
                id<MTLLibrary> defLib = [dev newDefaultLibrary];
                if (defLib == nil){
                    printf("Metal, error, newDefaultLibrary failed.\n");
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
                                ScnGpuDevice_release(&d);
                                ScnGpuDevice_null(&d);
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
            if(dev != nil){
                [dev release];
                dev = nil;
            }
        }
        if(devs != nil){
            [devs release];
            devs = nil;
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
    ScnContext_releaseAndNullify(&ctx);
}

//STScnApiMetalBuffer

typedef struct STScnApiMetalBuffer_ {
    ScnContextRef   ctx;
    STScnApiItf     itf;
    id<MTLDevice>   dev;
    id<MTLBuffer>   buff;
} STScnApiMetalBuffer;

ScnBOOL ScnApiMetal_buffer_syncAllRanges_(id<MTLBuffer> buff, ScnMemElasticRef mem);
        
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
                if(!ScnApiMetal_buffer_syncAllRanges_(buff, mem)){
                    printf("ERROR, ScnApiMetal_buffer_syncAllRanges_ failed.\n");
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
                            ScnGpuBuffer_release(&d);
                            ScnGpuBuffer_null(&d);
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
    ScnContext_releaseAndNullify(&ctx);
}

ScnBOOL ScnApiMetal_buffer_sync(void* pObj, const STScnGpuBufferCfg* cfg, ScnMemElasticRef mem, const STScnGpuBufferChanges* changes){
    ScnBOOL r = ScnFALSE;
    STScnApiMetalBuffer* obj = (STScnApiMetalBuffer*)pObj;
    //ToDo: sync only changes regions
    if(obj->buff == nil){
        printf("ERROR, ScnApiMetal_buffer_sync obj->buff is nil.\n");
    } else if(ScnMemElastic_isNull(mem)){
        printf("ERROR, ScnApiMetal_buffer_sync mem is NULL.\n");
    } else {
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
            }
        }
        //sync
        if(buffLen < cpuBuffSz){
            printf("ERROR, ScnApiMetal_buffer_sync::gpuBuff is smaller than cpu-buff.\n");
        } else if(!ScnApiMetal_buffer_syncAllRanges_(obj->buff, mem)){
            printf("ERROR, ScnApiMetal_buffer_sync::ScnApiMetal_buffer_syncAllRanges_ failed.\n");
        } else {
            r = ScnTRUE;
        }
    }
    return r;
}


ScnBOOL ScnApiMetal_buffer_syncAllRanges_(id<MTLBuffer> buff, ScnMemElasticRef mem){
    ScnBOOL r = ScnTRUE;
    STScnAbsPtr ptr = STScnAbsPtr_Zero;
    ScnUI32 iPos = 0, continuousSz = 0;
    const ScnUI32 buffLen = (ScnUI32)buff.length;
    ScnBYTE* buffPtr = (ScnBYTE*)buff.contents;
    do {
        ptr = ScnMemElastic_getNextContinuousAddress(mem, iPos, &continuousSz);
        if(ptr.ptr == NULL){
            r = (iPos == 0 ? ScnFALSE : ScnTRUE);
            break;
        }
        SCN_ASSERT(ptr.idx == iPos);
        SCN_ASSERT((iPos + continuousSz) <= buffLen);
        if((iPos + continuousSz) > buffLen){
            printf("ERROR, gpu-buffer is smaller than cpu-buffer.\n");
            r = ScnFALSE;
            break;
        }
        //copy
        {
            memcpy(&buffPtr[iPos], ptr.ptr, continuousSz);
        }
        iPos += continuousSz;
    } while(1);
    return r;
}

//STScnApiMetalVertexBuffer

typedef struct STScnApiMetalVertexBuff_ {
    ScnContextRef           ctx;
    STScnGpuVertexbuffCfg   cfg;
    STScnApiItf             itf;
    ScnGpuBufferRef         vBuff;
    ScnGpuBufferRef         idxBuff;
} STScnApiMetalVertexBuff;

ScnGpuVertexbuffRef ScnApiMetal_device_allocVertexBuff(void* pObj, const STScnGpuVertexbuffCfg* cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff){
    ScnGpuVertexbuffRef r = ScnObjRef_Zero;
    STScnApiMetalDevice* dev = (STScnApiMetalDevice*)pObj;
    if(dev != NULL && dev->dev != NULL && cfg != NULL && !ScnGpuBuffer_isNull(vBuff) && !ScnGpuBuffer_isNull(idxBuff)){
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
                ScnGpuVertexbuff_release(&d);
                ScnGpuVertexbuff_null(&d);
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
        ScnGpuBuffer_releaseAndNullify(&obj->vBuff);
        ScnGpuBuffer_releaseAndNullify(&obj->idxBuff);
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNullify(&ctx);
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

typedef struct STScnApiMetalFramebuffView_ {
    ScnContextRef       ctx;
    MTKView*            mtkView;
    STScnSizeU          size;
    STScnRectU          viewport;
    STScnApiItf         itf;
} STScnApiMetalFramebuffView;

ScnGpuFramebuffRef ScnApiMetal_device_allocFramebuffFromOSView(void* pObj, void* mtkView){
    ScnGpuFramebuffRef r = ScnObjRef_Zero;
    STScnApiMetalDevice* dev = (STScnApiMetalDevice*)pObj;
    if(dev != NULL && dev->dev != NULL && mtkView != nil){
        //synced
        STScnApiMetalFramebuffView* obj = (STScnApiMetalFramebuffView*)ScnContext_malloc(dev->ctx, sizeof(STScnApiMetalFramebuffView), "STScnApiMetalFramebuffView");
        if(obj == NULL){
            printf("ScnContext_malloc(STScnApiMetalFramebuffView) failed.\n");
        } else {
            memset(obj, 0, sizeof(*obj));
            ScnContext_set(&obj->ctx, dev->ctx);
            obj->itf        = dev->itf;
            obj->mtkView    = mtkView; [obj->mtkView retain];
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
                ScnGpuFramebuff_releaseAndNullify(&d);
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
        if(obj->mtkView != nil){
            [obj->mtkView release];
            obj->mtkView = nil;
        }
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNullify(&ctx);
}

STScnSizeU ScnApiMetal_framebuff_view_getSize(void* pObj, STScnRectU* dstViewport){
    STScnApiMetalFramebuffView* obj = (STScnApiMetalFramebuffView*)pObj;
    if(dstViewport != NULL) *dstViewport = obj->viewport;
    return obj->size;
}

ScnBOOL ScnApiMetal_framebuff_view_syncSizeAndViewport(void* pObj, const STScnSizeU size, const STScnRectU viewport){
    ScnBOOL r = ScnFALSE;
    STScnApiMetalFramebuffView* obj = (STScnApiMetalFramebuffView*)pObj;
    if(obj->mtkView != nil && size.width > 0 && size.height > 0){
        obj->size = size;
        obj->viewport = viewport;
        r = ScnTRUE;
    }
    return r;
}
