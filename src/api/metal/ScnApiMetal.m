//
//  ScnApiMetal.m
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/api/ScnApiMetal.h"
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

//STScnGpuDeviceApiItf

void*   ScnApiMetal_device_alloc(STScnContextRef ctx, const STScnGpuDeviceCfg* cfg, void* usrData);
void    ScnApiMetal_device_free(void* data, void* usrData);

ScnBOOL ScnApiMetal_getApiItf(STScnApiItf* dst){
    if(dst == NULL) return ScnFALSE;
    //
    memset(dst, 0, sizeof(*dst));
    //
    //device
    dst->dev.alloc      = ScnApiMetal_device_alloc;
    dst->dev.free       = ScnApiMetal_device_free;
    //
    return ScnTRUE;
}

//STScnApiMetalDevice

typedef struct STScnApiMetalDevice_ {
    STScnContextRef ctx;
    void*           usrData;
    //
    id<MTLDevice>   dev;
    id<MTLLibrary>  lib;
} STScnApiMetalDevice;

void* ScnApiMetal_device_alloc(STScnContextRef ctx, const STScnGpuDeviceCfg* cfg, void* usrData){
    void* r = NULL;
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
                        memset(obj, 0, sizeof(*obj));
                        ScnContext_set(&obj->ctx, ctx);
                        obj->usrData    = usrData;
                        obj->dev        = dev; [dev retain];
                        obj->lib        = defLib; [defLib retain];
                        r = obj;
                    }
                }
                //release (if not consumed)
                if(defLib != nil){
                    [defLib release];
                    defLib = nil;
                }
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

void ScnApiMetal_device_free(void* data, void* usrData){
    STScnApiMetalDevice* obj = (STScnApiMetalDevice*)data;
    STScnContextRef ctx = obj->ctx;
    if(obj->lib != nil){
        [obj->lib release];
        obj->lib = nil;
    }
    if(obj->dev != nil){
        [obj->dev release];
        obj->dev = nil;
    }
    ScnContext_null(&obj->ctx);
    ScnContext_mfree(ctx, obj);
    ScnContext_release(&ctx);
}
