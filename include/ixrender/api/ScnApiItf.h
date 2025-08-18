//
//  ScnApiItf.h
//  ixtli-render
//
//  Created by Marcos Ortega on 18/8/25.
//

#ifndef ScnApiItf_h
#define ScnApiItf_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/gpu/ScnGpuDevice.h"
#include "ixrender/gpu/ScnGpuBuffer.h"
#include "ixrender/gpu/ScnGpuVertexbuff.h"
#include "ixrender/gpu/ScnGpuTexture.h"
#include "ixrender/gpu/ScnGpuRenderbuff.h"
#include "ixrender/gpu/ScnGpuFramebuff.h"

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

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiItf_h */
