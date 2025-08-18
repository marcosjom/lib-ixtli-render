//
//  ScnApiMetalRenderJob.h
//  ixtli-render
//
//  Created by Marcos Ortega on 18/8/25.
//

#ifndef ScnApiMetalRenderJob_h
#define ScnApiMetalRenderJob_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnContext.h"
#include "ixrender/gpu/ScnGpuBuffer.h"
//
#include "ScnApiMetalDevice.h"
#include "ScnApiMetalBuffer.h"
#include "ScnApiMetalRenderJobState.h"
//
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import <TargetConditionals.h>  //for TARGET_OS_* macros

#ifdef __cplusplus
extern "C" {
#endif

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

void    ScnApiMetalRenderJob_free(void* data);
ENScnGpuRenderJobState ScnApiMetalRenderJob_getState(void* data);
ScnBOOL ScnApiMetalRenderJob_buildBegin(void* data, ScnGpuBufferRef bPropsScns, ScnGpuBufferRef bPropsMdls);
ScnBOOL ScnApiMetalRenderJob_buildAddCmds(void* data, const struct STScnRenderCmd* const cmds, const ScnUI32 cmdsSz);
ScnBOOL ScnApiMetalRenderJob_buildEndAndEnqueue(void* data);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiMetalRenderJob_h */
