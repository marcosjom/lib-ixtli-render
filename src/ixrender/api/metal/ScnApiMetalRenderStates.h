//
//  ScnApiMetalRenderStates.h
//  ixtli-render
//
//  Created by Marcos Ortega on 18/8/25.
//

#ifndef ScnApiMetalRenderStates_h
#define ScnApiMetalRenderStates_h

#include "ixrender/ixtli-defs.h"
//
#include "ScnApiMetalDevice.h"
//
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import <TargetConditionals.h>  //for TARGET_OS_* macros

#ifdef __cplusplus
extern "C" {
#endif

//STScnApiMetalRenderStates

#define ScnApiMetalRenderStates_ENScnVertexTypeFromVBuffCfg(VBUFF_CFG)  (VBUFF_CFG->texCoords[ENScnGpuTextureIdx_2].amm > 0 ? ENScnVertexType_2DTex3 : VBUFF_CFG->texCoords[ENScnGpuTextureIdx_1].amm > 0 ? ENScnVertexType_2DTex2 : VBUFF_CFG->texCoords[ENScnGpuTextureIdx_0].amm > 0 ? ENScnVertexType_2DTex : ENScnVertexType_2DColor)

typedef struct STScnApiMetalRenderStates {
    MTLPixelFormat              color;
    id<MTLRenderPipelineState>  states[ENScnVertexType_Count]; //shader and fragment for this framebuffer
} STScnApiMetalRenderStates;

void    STScnApiMetalRenderStates_init(STScnApiMetalRenderStates* obj);
void    STScnApiMetalRenderStates_destroy(STScnApiMetalRenderStates* obj);
ScnBOOL STScnApiMetalRenderStates_load(STScnApiMetalRenderStates* obj, STScnApiMetalDevice* dev, MTLPixelFormat color);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiMetalRenderStates_h */
