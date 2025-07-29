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

//STScnVertexbuffRef

SCN_REF_STRUCT_METHODS_DEC(ScnVertexbuff)

//

ScnBOOL         ScnVertexbuff_prepare(STScnVertexbuffRef ref, STScnGpuDeviceRef gpuDev, const ScnUI32 ammRenderSlots, const STScnGpuVertexbuffCfg* cfg, STScnBufferRef vertexBuff, STScnBufferRef idxsBuff);

ScnBOOL         ScnVertexbuff_activate(STScnVertexbuffRef ref);
ScnBOOL         ScnVertexbuff_deactivate(STScnVertexbuffRef ref);

ScnUI32         ScnVertexbuff_getSzPerRecord(STScnVertexbuffRef ref);
STScnBufferRef  ScnVertexbuff_getVertexBuff(STScnVertexbuffRef ref);
STScnBufferRef  ScnVertexbuff_getIdxsBuff(STScnVertexbuffRef ref);

//gpu-vertexbuffer
ScnBOOL         ScnVertexbuff_prepareNextRenderSlot(STScnVertexbuffRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnVertexbuff_h */
