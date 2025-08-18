//
//  ScnApiMetalDevice.m
//  ixtli-render
//
//  Created by Marcos Ortega on 18/8/25.
//

#include "ScnApiMetalDevice.h"
#include "ScnApiMetal.h"
#include "ScnApiMetalSampler.h"
#include "ScnApiMetalBuffer.h"
#include "ScnApiMetalVertexbuff.h"
#include "ScnApiMetalTexture.h"
#include "ScnApiMetalFramebuff.h"
#include "ScnApiMetalRenderJob.h"
//
#include "ixrender/gpu/ScnGpuSampler.h"

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
                    ScnApiMetalDevice_free(obj);
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

void ScnApiMetalDevice_free(void* pObj){
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

ScnGpuSamplerRef ScnApiMetalDevice_allocSampler(void* pObj, const STScnGpuSamplerCfg* const cfg){
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
                        itf.free    = ScnApiMetalSampler_free;
                        itf.getCfg  = ScnApiMetalSampler_getCfg;
                        if(!ScnGpuSampler_prepare(s, &itf, obj)){
                            SCN_PRINTF_ERROR("ScnApiMetalDevice_allocSampler::ScnGpuSampler_prepare failed.\n");
                        } else {
                            ScnGpuSampler_set(&r, s);
                            obj = NULL; //consume
                        }
                        ScnGpuSampler_releaseAndNull(&s);
                    }
                }
                //release (if not consumed)
                if(obj != NULL){
                    ScnApiMetalSampler_free(obj);
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

void* ScnApiMetalDevice_getApiDevice(void* pObj){
    STScnApiMetalDevice* dev = (STScnApiMetalDevice*)pObj;
    return (dev != NULL ? dev->dev : NULL);
}

STScnGpuDeviceDesc ScnApiMetalDevice_getDesc(void* obj){
    STScnGpuDeviceDesc r = STScnGpuDeviceDesc_Zero;
    {
        r.isTexFmtInfoRequired  = ScnTRUE;  //fragment shader requires the textures format info to produce correct color output
        r.offsetsAlign          = 32;       //buffers offsets aligment
        r.memBlockAlign         = 256;      //buffer memory copy alignment
    }
    return r;
}

ScnGpuBufferRef ScnApiMetalDevice_allocBuffer(void* pObj, ScnMemElasticRef mem){
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
                if(!ScnApiMetalBuffer_syncRanges_(buff, mem, &rngAll, 1)){
                    SCN_PRINTF_ERROR("ScnApiMetalBuffer_syncRanges_ failed.\n");
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
                        ScnApiMetalBuffer_free(obj);
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

ScnGpuVertexbuffRef ScnApiMetalDevice_allocVertexBuff(void* pObj, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff){
    ScnGpuVertexbuffRef r = ScnObjRef_Zero;
    STScnApiMetalDevice* dev = (STScnApiMetalDevice*)pObj;
    if(dev != NULL && dev->dev != NULL && cfg != NULL && !ScnGpuBuffer_isNull(vBuff)){ //idxBuff is optional
        //synced
        STScnApiMetalVertexbuff* obj = (STScnApiMetalVertexbuff*)ScnContext_malloc(dev->ctx, sizeof(STScnApiMetalVertexbuff), SCN_DBG_STR("STScnApiMetalVertexbuff"));
        if(obj == NULL){
            SCN_PRINTF_ERROR("ScnContext_malloc(STScnApiMetalVertexbuff) failed.\n");
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
                    SCN_PRINTF_ERROR("ScnApiMetalDevice_allocVertexBuff::ScnGpuVertexbuff_prepare failed.\n");
                } else {
                    ScnGpuVertexbuff_set(&r, d);
                    obj = NULL; //consume
                }
                ScnGpuVertexbuff_releaseAndNull(&d);
            }
        }
        //release (if not consumed)
        if(obj != NULL){
            ScnApiMetalVertexbuff_free(obj);
            obj = NULL;
        }
    }
    return r;
}

ScnGpuTextureRef ScnApiMetalDevice_allocTexture(void* pObj, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData){
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
                obj->sampler        = ScnApiMetalDevice_allocSampler(dev, &cfg->sampler);
                if(ScnGpuSampler_isNull(obj->sampler)){
                    SCN_PRINTF_ERROR("ScnApiMetalDevice_allocTexture::ScnApiMetalDevice_allocSampler failed.\n");
                } else {
                    ScnGpuTextureRef d = ScnGpuTexture_alloc(dev->ctx);
                    if(!ScnGpuTexture_isNull(d)){
                        STScnGpuTextureApiItf itf;
                        ScnMemory_setZeroSt(itf);
                        itf.free        = ScnApiMetalTexture_free;
                        itf.sync        = ScnApiMetalTexture_sync;
                        if(!ScnGpuTexture_prepare(d, &itf, obj)){
                            SCN_PRINTF_ERROR("ScnApiMetalDevice_allocTexture::ScnGpuTexture_prepare failed.\n");
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
                ScnApiMetalTexture_free(obj);
                obj = NULL;
            }
        }
    }
    return r;
}

ScnGpuFramebuffRef ScnApiMetalDevice_allocFramebuffFromOSView(void* pObj, void* pMtkView){
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
                itf.free        = ScnApiMetalFramebuff_view_free;
                itf.getSize     = ScnApiMetalFramebuff_view_getSize;
                itf.syncSize    = ScnApiMetalFramebuff_view_syncSize;
                itf.getProps    = ScnApiMetalFramebuff_view_getProps;
                itf.setProps    = ScnApiMetalFramebuff_view_setProps;
                if(!ScnGpuFramebuff_prepare(d, &itf, obj)){
                    SCN_PRINTF_ERROR("ScnApiMetalDevice_allocFramebuffFromOSView::ScnGpuFramebuff_prepare failed.\n");
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
            ScnApiMetalFramebuff_view_free(obj);
            obj = NULL;
        }
    }
    return r;
}


ScnGpuRenderJobRef ScnApiMetalDevice_allocRenderJob(void* pObj){
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
                itf.free        = ScnApiMetalRenderJob_free;
                itf.getState    = ScnApiMetalRenderJob_getState;
                itf.buildBegin  = ScnApiMetalRenderJob_buildBegin;
                itf.buildAddCmds = ScnApiMetalRenderJob_buildAddCmds;
                itf.buildEndAndEnqueue = ScnApiMetalRenderJob_buildEndAndEnqueue;
                //
                if(!ScnGpuRenderJob_prepare(d, &itf, obj)){
                    SCN_PRINTF_ERROR("ScnApiMetalDevice_allocRenderJob::ScnGpuRenderJob_prepare failed.\n");
                } else {
                    ScnGpuRenderJob_set(&r, d);
                    obj = NULL; //consume
                }
                ScnGpuRenderJob_releaseAndNull(&d);
            }
        }
        //release (if not consumed)
        if(obj != NULL){
            ScnApiMetalRenderJob_free(obj);
            obj = NULL;
        }
    }
    return r;
}
