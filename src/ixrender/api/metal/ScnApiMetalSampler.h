//
//  ScnApiMetalSampler.h
//  ixtli-render
//
//  Created by Marcos Ortega on 18/8/25.
//

#ifndef ScnApiMetalSampler_h
#define ScnApiMetalSampler_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnContext.h"
#include "ixrender/gpu/ScnGpuSampler.h"
//
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import <TargetConditionals.h>  //for TARGET_OS_* macros

#ifdef __cplusplus
extern "C" {
#endif

// STScnApiMetalSampler

typedef struct STScnApiMetalSampler {
    ScnContextRef           ctx;
    STScnGpuSamplerCfg      cfg;
    id<MTLSamplerState>     smplr;
} STScnApiMetalSampler;

void                ScnApiMetalSampler_free(void* pObj);
STScnGpuSamplerCfg  ScnApiMetalSampler_getCfg(void* data);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiMetalSampler_h */
