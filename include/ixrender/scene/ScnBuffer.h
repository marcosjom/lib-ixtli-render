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

typedef enum ENScnBufferType {
    ENScnBufferType_Static = 0, //static buffers have a cpu-buffer untill its synced to its gpu-buffer; the cpu-buffer is deleted then. Its content is expected to never change.
    ENScnBufferType_Dynamic,    //dynamic buffers have a cpu-buffer and one or multiple gpu-buffers for rendering; the gpu-buffer invalidated areas are synced from cpu-buffer on each render pass.
    //
    ENScnBufferType_Count
} ENScnBufferType;

//ScnBufferRef

#define ScnBufferRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnBuffer)

//

ScnBOOL     ScnBuffer_prepare(ScnBufferRef ref, ScnGpuDeviceRef gpuDev, const ScnUI32 ammRenderSlots, const STScnGpuBufferCfg* const cfg);
ScnBOOL     ScnBuffer_hasPtrs(ScnBufferRef ref); //allocations made?
ScnUI32     ScnBuffer_getRenderSlotsCount(ScnBufferRef ref);

//cpu-buffer
ScnBOOL     ScnBuffer_clear(ScnBufferRef ref);
ScnBOOL     ScnBuffer_invalidateAll(ScnBufferRef ref); //forces the full buffer to be synced to its gpu-buffer slot
STScnAbsPtr ScnBuffer_malloc(ScnBufferRef ref, const ScnUI32 usableSz);
ScnBOOL     ScnBuffer_mfree(ScnBufferRef ref, const STScnAbsPtr ptr);
ScnBOOL     ScnBuffer_mInvalidate(ScnBufferRef ref, const STScnAbsPtr ptr, const ScnUI32 sz);

//gpu-buffer
ScnBOOL         ScnBuffer_prepareCurrentRenderSlot(ScnBufferRef ref, ScnBOOL* dstHasPtrs);
ScnBOOL         ScnBuffer_moveToNextRenderSlot(ScnBufferRef ref);
ScnGpuBufferRef ScnBuffer_getCurrentRenderSlot(ScnBufferRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnBuffer_h */
