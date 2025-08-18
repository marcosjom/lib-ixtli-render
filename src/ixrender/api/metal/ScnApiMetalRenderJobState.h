//
//  ScnApiMetalRenderJobState.h
//  ixtli-render
//
//  Created by Marcos Ortega on 18/8/25.
//

#ifndef ScnApiMetalRenderJobState_h
#define ScnApiMetalRenderJobState_h

#include "ixrender/ixtli-defs.h"
//
#include "ScnApiMetalFramebuff.h"
//
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import <TargetConditionals.h>  //for TARGET_OS_* macros

#ifdef __cplusplus
extern "C" {
#endif

// STScnApiMetalRenderJobState

typedef struct STScnApiMetalRenderJobState {
    STScnApiMetalFramebuffView* fb;
    MTLRenderPassDescriptor*    rndrDesc; //per active framebuff
    id<MTLRenderCommandEncoder> rndrEnc; //per active framebuff
} STScnApiMetalRenderJobState;

void    ScnApiMetalRenderJobState_init(STScnApiMetalRenderJobState* obj);
void    ScnApiMetalRenderJobState_destroy(STScnApiMetalRenderJobState* obj);
//
void    ScnApiMetalRenderJobState_reset(STScnApiMetalRenderJobState* obj);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiMetalRenderJobState_h */
