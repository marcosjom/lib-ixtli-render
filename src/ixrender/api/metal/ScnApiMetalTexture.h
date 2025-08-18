//
//  ScnApiMetalTexture.h
//  ixtli-render
//
//  Created by Marcos Ortega on 18/8/25.
//

#ifndef ScnApiMetalTexture_h
#define ScnApiMetalTexture_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/gpu/ScnGpuSampler.h"
#include "ixrender/gpu/ScnGpuTexture.h"
#include "ixrender/api/ScnApiItf.h"
//
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import <TargetConditionals.h>  //for TARGET_OS_* macros

#ifdef __cplusplus
extern "C" {
#endif

// STScnApiMetalTexture

typedef struct STScnApiMetalTexture {
    ScnContextRef           ctx;
    STScnGpuTextureCfg      cfg;
    id<MTLTexture>          tex;
    ScnGpuSamplerRef        sampler;
    STScnApiItf             itf;
} STScnApiMetalTexture;

void    ScnApiMetalTexture_free(void* pObj);
ScnBOOL ScnApiMetalTexture_sync(void* data, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData, const STScnGpuTextureChanges* const changes);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiMetalTexture_h */
