//
//  ScnRender.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnRender_h
#define ScnRender_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/core/ScnCompare.h"
#include "ixrender/type/ScnRange.h"
#include "ixrender/gpu/ScnGpuBuffer.h"
#include "ixrender/gpu/ScnGpuVertexbuff.h"
#include "ixrender/gpu/ScnGpuTexture.h"
#include "ixrender/gpu/ScnGpuRenderbuff.h"
#include "ixrender/gpu/ScnGpuFramebuff.h"
#include "ixrender/gpu/ScnGpuDevice.h"
#include "ixrender/scene/ScnTransform.h"
#include "ixrender/scene/ScnBuffer.h"
#include "ixrender/scene/ScnVertexbuffs.h"
#include "ixrender/scene/ScnFramebuff.h"
#include "ixrender/scene/ScnRenderCmd.h"
#include "ixrender/scene/ScnModel.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnRenderApiItf

typedef struct STScnApiItf_ {
    ScnGpuDeviceRef           (*allocDevice)(ScnContextRef ctx, const STScnGpuDeviceCfg* cfg);
    STScnGpuDeviceApiItf        dev;    //device
    STScnGpuBufferApiItf        buff;   //buffers
    STScnGpuVertexbuffApiItf    vertexBuff;   //vertexBuff
    STScnGpuTextureApiItf       tex;    //textures
    STScnGpuRenderbuffApiItf    rbuff;  //render buffers
    STScnGpuFramebuffApiItf     fbuff;  //framebuffers
} STScnApiItf;

//ScnRenderRef

SCN_REF_STRUCT_METHODS_DEC(ScnRender)

//init
ScnBOOL     ScnRender_prepare(ScnRenderRef ref, const STScnApiItf* itf, void* itfParam);
ScnBOOL     ScnRender_openDevice(ScnRenderRef ref, const STScnGpuDeviceCfg* cfg, const ScnUI32 ammRenderSlots);
ScnBOOL     ScnRender_hasOpenDevice(ScnRenderRef ref);

//model
ScnModelRef ScnRender_allocModel(ScnRenderRef ref);

//framebuffer
ScnFramebuffRef ScnRender_allocFramebuff(ScnRenderRef ref);

//job

ScnBOOL     ScnRender_jobStart(ScnRenderRef ref);
ScnBOOL     ScnRender_jobEnd(ScnRenderRef ref);

//job framebuffers

ScnBOOL     ScnRender_jobFramebuffPush(ScnRenderRef ref, ScnFramebuffRef fbuff);
ScnBOOL     ScnRender_jobFramebuffPop(ScnRenderRef ref);

//job models

ScnBOOL     ScnRender_jobModelAdd(ScnRenderRef ref, ScnModelRef model);    //equivalent to push-and-pop
ScnBOOL     ScnRender_jobModelPush(ScnRenderRef ref, ScnModelRef model);
ScnBOOL     ScnRender_jobModelPop(ScnRenderRef ref);

//job cmds
/*
void        ScnRender_cmdMaskModePush(ScnRenderRef ref);
void        ScnRender_cmdMaskModePop(ScnRenderRef ref);
void        ScnRender_cmdSetTexture(ScnRenderRef ref, const ScnUI32 index, ScnTextureRef tex);
void        ScnRender_cmdSetVertsType(ScnRenderRef ref, const ENScnVertexType type);
void        ScnRender_cmdDawVerts(ScnRenderRef ref, const ENScnRenderShape mode, const ScnUI32 iFirst, const ScnUI32 count);
void        ScnRender_cmdDawIndexes(ScnRenderRef ref, const ENScnRenderShape mode, const ScnUI32 iFirst, const ScnUI32 count);
*/

//custom actions (for custom shaders)

ScnVertexbuffsRef   ScnRender_getDefaultVertexbuffs(ScnRenderRef ref);
ScnVertexbuffsRef   ScnRender_allocVertexbuffs(ScnRenderRef ref);
ScnBufferRef        ScnRender_allocBuffer(ScnRenderRef ref, const STScnGpuBufferCfg* cfg);
//
ScnBOOL             ScnRender_jobTransformPush(ScnRenderRef ref, STScnModelProps* t);
ScnBOOL             ScnRender_jobTransformPop(ScnRenderRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRender_h */
