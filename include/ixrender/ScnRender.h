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
#include "ixrender/scene/ScnVertexbuffs.h"
#include "ixrender/scene/ScnRenderCmd.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnRenderNode

#define STScnRenderNode_Zero  { 0, ScnFALSE, 0, STScnTransform_Zero, STScnRangeI_Zero }

typedef struct STScnRenderNode_ {
    ScnUI16         iDepth;     //level in the tree
    ScnBOOL         isPopped;   //node is still open (pushed but not popped yet)
    ScnUI32         underCount; //ammount of nodes affected by this (children and below)
    STScnTransform  tform;      //transform properties
    STScnRangeI     cmds;       //range of commands
} STScnRenderNode;

//ScnRenderApiItf

typedef struct STScnApiItf_ {
    STScnGpuDeviceApiItf        dev;    //device
    STScnGpuBufferApiItf        buff;   //buffers
    STScnGpuVertexbuffApiItf    vertexBuff;   //vertexBuff
    STScnGpuTextureApiItf       tex;    //textures
    STScnGpuRenderbuffApiItf    rbuff;  //render buffers
    STScnGpuFramebuffApiItf     fbuff;  //framebuffers
} STScnApiItf;

//STScnRenderRef

SCN_REF_STRUCT_METHODS_DEC(ScnRender)

//Prepare

ScnBOOL ScnRender_prepare(STScnRenderRef ref, const STScnApiItf* itf, void* itfParam);

//Device

ScnBOOL ScnRender_openDevice(STScnRenderRef ref, const STScnGpuDeviceCfg* cfg);

//Vertices

STScnVertexbuffsRef ScnRender_getDefaultVertexbuffs(STScnRenderRef ref);
ScnBOOL     ScnRender_createVertexbuffs(STScnRenderRef ref, STScnVertexbuffsRef* dst);

//Buffers

ScnUI32     ScnRender_bufferCreate(STScnRenderRef ref, const STScnGpuBufferCfg* cfg);     //allocates a new buffer
ScnBOOL     ScnRender_bufferDestroy(STScnRenderRef ref, const ScnUI32 bid);                 //flags a buffer for destruction

//job

void        ScnRender_jobStart(STScnRenderRef ref);
ScnBOOL     ScnRender_jobEnd(STScnRenderRef ref);

//job nodes

void        ScnRender_nodePush(STScnRenderRef ref, const STScnTransform* tform);
ScnBOOL     ScnRender_nodePop(STScnRenderRef ref);

//job cmds

void        ScnRender_cmdMaskModePush(STScnRenderRef ref);
void        ScnRender_cmdMaskModePop(STScnRenderRef ref);
void        ScnRender_cmdSetTexture(STScnRenderRef ref, const ScnUI32 index, const ScnUI32 tex /*const STScnGpuTextureRef tex*/);
void        ScnRender_cmdSetVertsType(STScnRenderRef ref, const ENScnVertexType type);
void        ScnRender_cmdDawVerts(STScnRenderRef ref, const ENScnRenderShape mode, const ScnUI32 iFirst, const ScnUI32 count);
void        ScnRender_cmdDawIndexes(STScnRenderRef ref, const ENScnRenderShape mode, const ScnUI32 iFirst, const ScnUI32 count);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRender_h */
