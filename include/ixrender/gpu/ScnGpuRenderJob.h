//
//  ScnGpuRenderJob.h
//  ixtli-render
//
//  Created by Marcos Ortega on 10/8/25.
//

#ifndef ScnGpuRenderJob_h
#define ScnGpuRenderJob_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/gpu/ScnGpuBuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

struct STScnRenderCmd; //external

//ENScnGpuRenderJobState

typedef enum ENScnGpuRenderJobState {
    ENScnGpuRenderJobState_Unknown = 0  //unknown
    //
    , ENScnGpuRenderJobState_Enqueued   //added to queue, active
    , ENScnGpuRenderJobState_Completed  //removed from queue, succesfully completed
    , ENScnGpuRenderJobState_Error      //removed from queue, with error status
    //
    , ENScnGpuRenderJobState_Count
} ENScnGpuRenderJobState;

//STScnGpuRenderJobApiItf

typedef struct STScnGpuRenderJobApiItf {
    void                    (*free)(void* data);
    ENScnGpuRenderJobState  (*getState)(void* data);
    ScnBOOL                 (*buildBegin)(void* data, ScnGpuBufferRef bPropsScns, ScnGpuBufferRef bPropsMdls);
    ScnBOOL                 (*buildAddCmds)(void* data, const struct STScnRenderCmd* const cmds, const ScnUI32 cmdsSz);
    ScnBOOL                 (*buildEndAndEnqueue)(void* data);
} STScnGpuRenderJobApiItf;


//ScnGpuRenderJobRef

#define ScnGpuRenderJobRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnGpuRenderJob)

//

ScnBOOL                 ScnGpuRenderJob_prepare(ScnGpuRenderJobRef ref, const STScnGpuRenderJobApiItf* itf, void* itfParam);
void*                   ScnGpuRenderJob_getApiItfParam(ScnGpuRenderJobRef ref);

ENScnGpuRenderJobState  ScnGpuRenderJob_getState(ScnGpuRenderJobRef ref);
ScnBOOL                 ScnGpuRenderJob_buildBegin(ScnGpuRenderJobRef ref, ScnGpuBufferRef bPropsScns, ScnGpuBufferRef bPropsMdls);
ScnBOOL                 ScnGpuRenderJob_buildAddCmds(ScnGpuRenderJobRef ref, const struct STScnRenderCmd* const cmds, const ScnUI32 cmdsSz);
ScnBOOL                 ScnGpuRenderJob_buildEndAndEnqueue(ScnGpuRenderJobRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuRenderJob_h */
