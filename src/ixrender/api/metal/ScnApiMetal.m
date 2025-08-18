//
//  ScnApiMetal.m
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/api/ScnApiMetal.h"
#include "ixrender/gpu/ScnGpuDevice.h"
#include "ixrender/gpu/ScnGpuBuffer.h"
#include "ixrender/gpu/ScnGpuSampler.h"
#include "ixrender/gpu/ScnGpuTexture.h"
#include "ixrender/scene/ScnRenderCmd.h"
//

#include "ScnApiMetalDevice.h"
#include "ScnApiMetalSampler.h"
#include "ScnApiMetalBuffer.h"
#include "ScnApiMetalVertexbuff.h"
#include "ScnApiMetalTexture.h"
#include "ScnApiMetalRenderStates.h"
#include "ScnApiMetalFramebuff.h"
#include "ScnApiMetalRenderJobState.h"
#include "ScnApiMetalRenderJob.h"

ScnBOOL ScnApiMetal_getApiItf(STScnApiItf* dst){
    if(dst == NULL) return ScnFALSE;
    //
    ScnMemory_setZeroSt(*dst);
    //gobal
    dst->allocDevice        = ScnApiMetal_allocDevice;
    //device
    dst->dev.free           = ScnApiMetalDevice_free;
    dst->dev.getApiDevice   = ScnApiMetalDevice_getApiDevice;
    dst->dev.getDesc        = ScnApiMetalDevice_getDesc;
    dst->dev.allocBuffer    = ScnApiMetalDevice_allocBuffer;
    dst->dev.allocVertexBuff = ScnApiMetalDevice_allocVertexBuff;
    dst->dev.allocFramebuffFromOSView = ScnApiMetalDevice_allocFramebuffFromOSView;
    dst->dev.allocTexture   = ScnApiMetalDevice_allocTexture;
    dst->dev.allocSampler   = ScnApiMetalDevice_allocSampler;
    dst->dev.allocRenderJob = ScnApiMetalDevice_allocRenderJob;
    //buffer
    dst->buff.free          = ScnApiMetalBuffer_free;
    dst->buff.sync          = ScnApiMetalBuffer_sync;
    //vertexbuff
    dst->vertexBuff.free    = ScnApiMetalVertexbuff_free;
    dst->vertexBuff.sync    = ScnApiMetalVertexbuff_sync;
    dst->vertexBuff.activate = ScnApiMetalVertexbuff_activate;
    dst->vertexBuff.deactivate = ScnApiMetalVertexbuff_deactivate;
    //
    return ScnTRUE;
}






