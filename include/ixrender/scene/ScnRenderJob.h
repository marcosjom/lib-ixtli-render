//
//  ScnRenderJob.h
//  ixtli-render
//
//  Created by Marcos Ortega on 10/8/25.
//

#ifndef ScnRenderJob_h
#define ScnRenderJob_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/type/ScnAABBox.h"
#include "ixrender/gpu/ScnGpuRenderJob.h"
#include "ixrender/gpu/ScnGpuFramebuffProps.h"
#include "ixrender/scene/ScnNode2d.h"
#include "ixrender/scene/ScnModel2d.h"
#include "ixrender/scene/ScnFramebuff.h"
#include "ixrender/scene/ScnRenderCmds.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ScnRenderJobPushMode_ {
    ScnRenderJobPushMode_Multiply = 0,  //Default, multiplies the input with the parent's state (inputs are relative properties)
    ScnRenderJobPushMode_Set,           //Ignores the parent's state and sets the input (inputs are global properties)
    //
    ScnRenderJobPushMode_Count
} ScnRenderJobPushMode;

//ScnRenderJobRef

#define ScnRenderJobRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnRenderJob)

//state

ENScnGpuRenderJobState  ScnRenderJob_getState(ScnRenderJobRef ref);

// internal buffers (optimization to reduce memory allocations between render jobs)

ScnBOOL     ScnRenderJob_swapCmds(ScnRenderJobRef ref, STScnRenderCmds* srcAndDstCmds, ScnGpuRenderJobRef* srcAndDestGpuJob);

// framebuffers

ScnBOOL     ScnRenderJob_framebuffPush(ScnRenderJobRef ref, ScnFramebuffRef fbuff);
ScnBOOL     ScnRenderJob_framebuffPop(ScnRenderJobRef ref);

ScnBOOL     ScnRenderJob_framebuffPropsPush(ScnRenderJobRef ref, const STScnGpuFramebuffProps props);
ScnBOOL     ScnRenderJob_framebuffPropsOrthoPush(ScnRenderJobRef ref, const STScnAABBox3d ortho);
ScnBOOL     ScnRenderJob_framebuffPropsPop(ScnRenderJobRef ref);

// nodes (scene's tree)

ScnBOOL     ScnRenderJob_node2dPush(ScnRenderJobRef ref, ScnNode2dRef node);
ScnBOOL     ScnRenderJob_node2dPushWithMode(ScnRenderJobRef ref, ScnNode2dRef node, const ScnRenderJobPushMode mode);
ScnBOOL     ScnRenderJob_node2dPropsPush(ScnRenderJobRef ref, const STScnNode2dProps nodeProps);
ScnBOOL     ScnRenderJob_node2dPropsPushWithMode(ScnRenderJobRef ref, const STScnNode2dProps nodeProps, const ScnRenderJobPushMode mode);
ScnBOOL     ScnRenderJob_node2dPop(ScnRenderJobRef ref);

// models

ScnBOOL     ScnRenderJob_model2dAdd(ScnRenderJobRef ref, ScnModel2dRef model);
ScnBOOL     ScnRenderJob_model2dAddWithNode(ScnRenderJobRef ref, ScnModel2dRef model, ScnNode2dRef node);                      //equivalent to    jobNode2dPush() + jobModelAdd() + jobNodePropsPop()
ScnBOOL     ScnRenderJob_model2dAddWithNodeAndMode(ScnRenderJobRef ref, ScnModel2dRef model, ScnNode2dRef node, const ScnRenderJobPushMode mode);
ScnBOOL     ScnRenderJob_model2dAddWithNodeProps(ScnRenderJobRef ref, ScnModel2dRef model, const STScnNode2dProps nodeProps);  //equivalent to jobNodePropsPush() + jobModelAdd() + jobNodePropsPop()
ScnBOOL     ScnRenderJob_model2dAddWithNodePropsAndMode(ScnRenderJobRef ref, ScnModel2dRef model, const STScnNode2dProps nodeProps, const ScnRenderJobPushMode mode);


// cmds
/*
void        ScnRenderJob_cmdMaskModePush(ScnRenderJobRef ref);
void        ScnRenderJob_cmdMaskModePop(ScnRenderJobRef ref);
void        ScnRenderJob_cmdSetTexture(ScnRenderJobRef ref, const ScnUI32 index, ScnTextureRef tex);
void        ScnRenderJob_cmdDawVerts(ScnRenderJobRef ref, const ENScnRenderShape mode, const ScnUI32 iFirst, const ScnUI32 count);
void        ScnRenderJob_cmdDawIndexes(ScnRenderJobRef ref, const ENScnRenderShape mode, const ScnUI32 iFirst, const ScnUI32 count);
*/

#ifdef __cplusplus
} //extern "C"
#endif


#endif /* ScnRenderJob_h */
