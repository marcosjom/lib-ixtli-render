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
#include "ixrender/scene/ScnTransform2D.h"
#include "ixrender/scene/ScnBuffer.h"
#include "ixrender/scene/ScnVertexbuffs.h"
#include "ixrender/scene/ScnFramebuff.h"
#include "ixrender/scene/ScnRenderCmd.h"
#include "ixrender/scene/ScnNode2d.h"
#include "ixrender/scene/ScnModel2d.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnRenderApiItf

typedef struct STScnApiItf {
    ScnGpuDeviceRef             (*allocDevice)(ScnContextRef ctx, const STScnGpuDeviceCfg* cfg);
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
void*       ScnRender_getApiDevice(ScnRenderRef ref);

//model
ScnModel2dRef ScnRender_allocModel(ScnRenderRef ref);

//framebuffer
ScnFramebuffRef ScnRender_allocFramebuff(ScnRenderRef ref);

//job

ScnBOOL     ScnRender_jobStart(ScnRenderRef ref);
ScnBOOL     ScnRender_jobEnd(ScnRenderRef ref); //does the magic

//job framebuffers

ScnBOOL     ScnRender_jobFramebuffPush(ScnRenderRef ref, ScnFramebuffRef fbuff);
ScnBOOL     ScnRender_jobFramebuffPop(ScnRenderRef ref);

//job nodes (scene's tree)

ScnBOOL     ScnRender_jobNode2dPush(ScnRenderRef ref, ScnNode2dRef node);
ScnBOOL     ScnRender_jobNode2dPropsPush(ScnRenderRef ref, const STScnNode2dProps nodeProps);
ScnBOOL     ScnRender_jobNode2dPop(ScnRenderRef ref);

//job models

ScnBOOL     ScnRender_jobModel2dAddWithNode(ScnRenderRef ref, ScnModel2dRef model, ScnNode2dRef node);                      //equivalent to    jobNode2dPush() + jobModelAdd() + jobNodePropsPop()
ScnBOOL     ScnRender_jobModel2dAddWithNodeProps(ScnRenderRef ref, ScnModel2dRef model, const STScnNode2dProps nodeProps);  //equivalent to jobNodePropsPush() + jobModelAdd() + jobNodePropsPop()
ScnBOOL     ScnRender_jobModel2dAdd(ScnRenderRef ref, ScnModel2dRef model);

//job cmds
/*
void        ScnRender_cmdMaskModePush(ScnRenderRef ref);
void        ScnRender_cmdMaskModePop(ScnRenderRef ref);
void        ScnRender_cmdSetTexture(ScnRenderRef ref, const ScnUI32 index, ScnTextureRef tex);
void        ScnRender_cmdDawVerts(ScnRenderRef ref, const ENScnRenderShape mode, const ScnUI32 iFirst, const ScnUI32 count);
void        ScnRender_cmdDawIndexes(ScnRenderRef ref, const ENScnRenderShape mode, const ScnUI32 iFirst, const ScnUI32 count);
*/

//custom actions (for custom shaders)

ScnVertexbuffsRef   ScnRender_getDefaultVertexbuffs(ScnRenderRef ref);
ScnVertexbuffsRef   ScnRender_allocVertexbuffs(ScnRenderRef ref);
ScnBufferRef        ScnRender_allocBuffer(ScnRenderRef ref, const STScnGpuBufferCfg* cfg);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRender_h */
