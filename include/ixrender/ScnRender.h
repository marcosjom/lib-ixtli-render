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
#include "ixrender/scene/ScnResourceMode.h"
#include "ixrender/scene/ScnTransform2d.h"
#include "ixrender/scene/ScnBuffer.h"
#include "ixrender/scene/ScnVertexbuffs.h"
#include "ixrender/scene/ScnFramebuff.h"
#include "ixrender/scene/ScnRenderCmd.h"
#include "ixrender/scene/ScnNode2d.h"
#include "ixrender/scene/ScnModel2d.h"
#include "ixrender/scene/ScnRenderJob.h"

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

#define ScnRenderRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnRender)

//init
ScnBOOL             ScnRender_prepare(ScnRenderRef ref, const STScnApiItf* itf, void* itfParam);
ScnBOOL             ScnRender_openDevice(ScnRenderRef ref, const STScnGpuDeviceCfg* cfg, const ScnUI32 ammRenderSlots);
ScnBOOL             ScnRender_hasOpenDevice(ScnRenderRef ref);
void*               ScnRender_getApiDevice(ScnRenderRef ref);
STScnGpuDeviceDesc  ScnRender_getDeviceDesc(ScnRenderRef ref);

//model

ScnModel2dRef       ScnRender_allocModel(ScnRenderRef ref);

//framebuffer

ScnFramebuffRef     ScnRender_allocFramebuff(ScnRenderRef ref);

//texture

ScnTextureRef       ScnRender_allocTexture(ScnRenderRef ref, const ENScnResourceMode mode, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const optSrcProps, const void* optSrcData);

//sampler

ScnGpuSamplerRef    ScnRender_allocSampler(ScnRenderRef ref, const STScnGpuSamplerCfg* const cfg);

//renderJob

ScnRenderJobRef     ScnRender_allocRenderJob(ScnRenderRef ref); //if render slot is available
ScnBOOL             ScnRender_enqueue(ScnRenderRef ref, ScnRenderJobRef job);

//custom actions (for custom shaders)

ScnVertexbuffsRef   ScnRender_getDefaultVertexbuffs(ScnRenderRef ref);
ScnVertexbuffsRef   ScnRender_allocVertexbuffs(ScnRenderRef ref);
ScnBufferRef        ScnRender_allocBuffer(ScnRenderRef ref, const STScnGpuBufferCfg* const cfg);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRender_h */
