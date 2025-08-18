//
//  ScnApiMetalVertexbuff.h
//  ixtli-render
//
//  Created by Marcos Ortega on 18/8/25.
//

#ifndef ScnApiMetalVertexbuff_h
#define ScnApiMetalVertexbuff_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnContext.h"
#include "ixrender/gpu/ScnGpuVertexbuff.h"
#include "ixrender/gpu/ScnGpuBuffer.h"
#include "ixrender/api/ScnApiItf.h"
//
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import <TargetConditionals.h>  //for TARGET_OS_* macros

#ifdef __cplusplus
extern "C" {
#endif

//STScnApiMetalVertexbuffer

typedef struct STScnApiMetalVertexbuff {
    ScnContextRef           ctx;
    STScnGpuVertexbuffCfg   cfg;
    STScnApiItf             itf;
    ScnGpuBufferRef         vBuff;
    ScnGpuBufferRef         idxBuff;
} STScnApiMetalVertexbuff;

void    ScnApiMetalVertexbuff_free(void* data);
ScnBOOL ScnApiMetalVertexbuff_sync(void* data, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff);
ScnBOOL ScnApiMetalVertexbuff_activate(void* data);
ScnBOOL ScnApiMetalVertexbuff_deactivate(void* data);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiMetalVertexbuff_h */
