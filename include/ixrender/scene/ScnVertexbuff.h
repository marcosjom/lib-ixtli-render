//
//  ScnVertexbuff.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnVertexbuff_h
#define ScnVertexbuff_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/scene/ScnBuffer.h"
#include "ixrender/gpu/ScnGpuDevice.h"
#include "ixrender/gpu/ScnGpuVertexbuff.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnVertexbuffRef

SCN_REF_STRUCT_METHODS_DEC(ScnVertexbuff)

//

ScnBOOL         ScnVertexbuff_prepare(ScnVertexbuffRef ref, ScnGpuDeviceRef gpuDev, const ScnUI32 ammRenderSlots, const STScnGpuVertexbuffCfg* const cfg, ScnBufferRef vertexBuff, ScnBufferRef idxsBuff);

ScnBOOL         ScnVertexbuff_activate(ScnVertexbuffRef ref);
ScnBOOL         ScnVertexbuff_deactivate(ScnVertexbuffRef ref);

ScnUI32         ScnVertexbuff_getSzPerRecord(ScnVertexbuffRef ref);
ScnBufferRef    ScnVertexbuff_getVertexBuff(ScnVertexbuffRef ref);
ScnBufferRef    ScnVertexbuff_getIdxsBuff(ScnVertexbuffRef ref);

//gpu-vertexbuffer
ScnBOOL         ScnVertexbuff_prepareCurrentRenderSlot(ScnVertexbuffRef ref);
ScnBOOL         ScnVertexbuff_moveToNextRenderSlot(ScnVertexbuffRef ref);
ScnGpuVertexbuffRef ScnVertexbuff_getCurrentRenderSlot(ScnVertexbuffRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnVertexbuff_h */
