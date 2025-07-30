//
//  ScnBuffer.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnBuffer_h
#define ScnBuffer_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/core/ScnMemElastic.h"
#include "ixrender/type/ScnRange.h"
#include "ixrender/scene/ScnVertices.h"
#include "ixrender/gpu/ScnGpuDevice.h"
#include "ixrender/gpu/ScnGpuBuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

//ENScnBufferType

typedef enum ENScnBufferType_ {
    ENScnBufferType_Static = 0, //static buffers have a cpu-buffer untill its synced to its gpu-buffer; the cpu-buffer is deleted then. Its content is expected to never change.
    ENScnBufferType_Dynamic,    //dynamic buffers have a cpu-buffer and one or multiple gpu-buffers for rendering; the gpu-buffer invalidated areas are synced from cpu-buffer on each render pass.
    //
    ENScnBufferType_Count
} ENScnBufferType;

//STScnBufferRef

SCN_REF_STRUCT_METHODS_DEC(ScnBuffer)

//

ScnBOOL     ScnBuffer_prepare(STScnBufferRef ref, STScnGpuDeviceRef gpuDev, const ScnUI32 ammRenderSlots, const STScnGpuBufferCfg* cfg);
ScnBOOL     ScnBuffer_hasPtrs(STScnBufferRef ref); //allocations made?
ScnUI32     ScnBuffer_getRenderSlotsCount(STScnBufferRef ref);

//cpu-buffer
ScnBOOL     ScnBuffer_clear(STScnBufferRef ref);
ScnBOOL     ScnBuffer_invalidateAll(STScnBufferRef ref); //forces the full buffer to be synced to its gpu-buffer slot
STScnAbsPtr ScnBuffer_malloc(STScnBufferRef ref, const ScnUI32 usableSz);
ScnBOOL     ScnBuffer_mfree(STScnBufferRef ref, const STScnAbsPtr ptr);
ScnBOOL     ScnBuffer_mInvalidate(STScnBufferRef ref, const STScnAbsPtr ptr, const ScnUI32 sz);

//gpu-buffer
ScnBOOL             ScnBuffer_prepareNextRenderSlot(STScnBufferRef ref, ScnBOOL* dstHasPtrs);
STScnGpuBufferRef   ScnBuffer_getCurrentRenderSlotGpuBuffer(STScnBufferRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnBuffer_h */
