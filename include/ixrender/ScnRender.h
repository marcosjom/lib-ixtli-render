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
    STScnGpuDeviceRef           (*allocDevice)(STScnContextRef ctx, const STScnGpuDeviceCfg* cfg);
    STScnGpuDeviceApiItf        dev;    //device
    STScnGpuBufferApiItf        buff;   //buffers
    STScnGpuVertexbuffApiItf    vertexBuff;   //vertexBuff
    STScnGpuTextureApiItf       tex;    //textures
    STScnGpuRenderbuffApiItf    rbuff;  //render buffers
    STScnGpuFramebuffApiItf     fbuff;  //framebuffers
} STScnApiItf;

//STScnRenderRef

SCN_REF_STRUCT_METHODS_DEC(ScnRender)

//init
ScnBOOL     ScnRender_prepare(STScnRenderRef ref, const STScnApiItf* itf, void* itfParam);
ScnBOOL     ScnRender_openDevice(STScnRenderRef ref, const STScnGpuDeviceCfg* cfg, const ScnUI32 ammRenderSlots);

//models
STScnModelRef ScnRender_allocModel(STScnRenderRef ref);

//job

ScnBOOL     ScnRender_jobStart(STScnRenderRef ref);
ScnBOOL     ScnRender_jobEnd(STScnRenderRef ref);

//job framebuffers

ScnBOOL     ScnRender_jobFramebuffPush(STScnRenderRef ref, STScnFramebuffRef fbuff);
ScnBOOL     ScnRender_jobFramebuffPop(STScnRenderRef ref);

//job transforms

ScnBOOL     ScnRender_jobTransformPush(STScnRenderRef ref, STScnModelProps* t);
ScnBOOL     ScnRender_jobTransformPop(STScnRenderRef ref);


//job cmds

void        ScnRender_cmdMaskModePush(STScnRenderRef ref);
void        ScnRender_cmdMaskModePop(STScnRenderRef ref);
void        ScnRender_cmdSetTexture(STScnRenderRef ref, const ScnUI32 index, const ScnUI32 tex /*const STScnGpuTextureRef tex*/);
void        ScnRender_cmdSetVertsType(STScnRenderRef ref, const ENScnVertexType type);
void        ScnRender_cmdDawVerts(STScnRenderRef ref, const ENScnRenderShape mode, const ScnUI32 iFirst, const ScnUI32 count);
void        ScnRender_cmdDawIndexes(STScnRenderRef ref, const ENScnRenderShape mode, const ScnUI32 iFirst, const ScnUI32 count);

//gpu-render

ScnBOOL     ScnRender_prepareNextRenderSlot(STScnRenderRef ref);

//custom actions (for custom shaders)

STScnVertexbuffsRef ScnRender_getDefaultVertexbuffs(STScnRenderRef ref);
STScnVertexbuffsRef ScnRender_allocVertexbuffs(STScnRenderRef ref);
STScnBufferRef      ScnRender_allocBuffer(STScnRenderRef ref, const STScnGpuBufferCfg* cfg);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRender_h */
