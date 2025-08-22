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
#include "ixrender/scene/ScnRenderJobPushMode.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnRenderJobRef

/** @struct ScnRenderJobRef
 *  @brief ScnRenderJob shared pointer. An object that contains commands to render a full or partial scene, including a retained map of the used objects (textures, buffers, ...).
 */

#define ScnRenderJobRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnRenderJob)

//state

/**
 * @brief Retrieves the current state of the render job on the gpu side.
 * @param ref Reference to object.
 * @return The state on gpu side on success, ENScnGpuRenderJobState_Unknown otherwise.
 */
ENScnGpuRenderJobState ScnRenderJob_getState(ScnRenderJobRef ref);

/**
 * @brief Swaps the internal commands container with the provided one. This allows the reutilization of commands container buffers.
 * @param ref Reference to object.
 * @param srcAndDstCmds Commands container to swap from and to.
 * @param srcAndDestGpuJob Gpu render job to swap from and to.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRenderJob_swapCmds(ScnRenderJobRef ref, STScnRenderCmds* srcAndDstCmds, ScnGpuRenderJobRef* srcAndDestGpuJob);

// framebuffers

/**
 * @brief Adds a new framebuffer to the top of the stack.
 * @param ref Reference to object.
 * @param fbuff Reference to framebuffer.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRenderJob_framebuffPush(ScnRenderJobRef ref, ScnFramebuffRef fbuff);

/**
 * @brief Removes the top framebuffer from the stack.
 * @param ref Reference to object.
 * @param fbuff Reference to framebuffer.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRenderJob_framebuffPop(ScnRenderJobRef ref);

/**
 * @brief Adds a new scene properties to the top of the framebuffer's stack.
 * @param ref Reference to object.
 * @param props Properties.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRenderJob_framebuffPropsPush(ScnRenderJobRef ref, const STScnGpuFramebuffProps props);

/**
 * @brief Adds a new scene orthogonal box to the top of the framebuffer's stack.
 * @param ref Reference to object.
 * @param ortho Orthogonal box.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRenderJob_framebuffPropsOrthoPush(ScnRenderJobRef ref, const STScnAABBox3d ortho);

/**
 * @brief Removes the top scene properties from the framebuffer's stack.
 * @param ref Reference to object.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRenderJob_framebuffPropsPop(ScnRenderJobRef ref);

// nodes (scene's tree)

/**
 * @brief Adds a new model properties to the top of the framebuffer's stack; multiplication is applied.
 * @param ref Reference to object.
 * @param node Node with properties.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRenderJob_node2dPush(ScnRenderJobRef ref, ScnNode2dRef node);

/**
 * @brief Adds a new model properties to the top of the framebuffer's stack with the specified mode.
 * @param ref Reference to object.
 * @param node Node with properties.
 * @param mode Adding mode..
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRenderJob_node2dPushWithMode(ScnRenderJobRef ref, ScnNode2dRef node, const ScnRenderJobPushMode mode);

/**
 * @brief Adds a new model properties to the top of the framebuffer's stack; multiplication is applied.
 * @param ref Reference to object.
 * @param nodeProps Node properties.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRenderJob_node2dPropsPush(ScnRenderJobRef ref, const STScnNode2dProps nodeProps);

/**
 * @brief Adds a new model properties to the top of the framebuffer's stack with the specified mode.
 * @param ref Reference to object.
 * @param nodeProps Node properties.
 * @param mode Adding mode..
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRenderJob_node2dPropsPushWithMode(ScnRenderJobRef ref, const STScnNode2dProps nodeProps, const ScnRenderJobPushMode mode);

/**
 * @brief Removes the top model properties from the framebuffer's stack.
 * @param ref Reference to object.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRenderJob_node2dPop(ScnRenderJobRef ref);

// models

/**
 * @brief Adds a model render commands into the commands container, multiplying with the top model properties at the stack.
 * @param ref Reference to object.
 * @param model Model to add.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRenderJob_model2dAdd(ScnRenderJobRef ref, ScnModel2dRef model);

/**
 * @brief Adds a model render commands into the commands container, multiplying the provided node with the top model properties at the stack.
 * @note Equivalent to jobNode2dPush() + jobModelAdd() + jobNodePropsPop().
 * @param ref Reference to object.
 * @param model Model to add.
 * @param node Model properties to apply.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRenderJob_model2dAddWithNode(ScnRenderJobRef ref, ScnModel2dRef model, ScnNode2dRef node);

/**
 * @brief Adds a model render commands into the commands container, using the provided node and mode, with the top model properties at the stack.
 * @param ref Reference to object.
 * @param model Model to add.
 * @param node Model properties to apply.
 * @param mode Node properties push mode.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRenderJob_model2dAddWithNodeAndMode(ScnRenderJobRef ref, ScnModel2dRef model, ScnNode2dRef node, const ScnRenderJobPushMode mode);

/**
 * @brief Adds a model render commands into the commands container, multiplying the provided node properties with the top model properties at the stack.
 * @note Equivalent to jobNodePropsPush() + jobModelAdd() + jobNodePropsPop().
 * @param ref Reference to object.
 * @param model Model to add.
 * @param nodeProps Model properties to apply.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRenderJob_model2dAddWithNodeProps(ScnRenderJobRef ref, ScnModel2dRef model, const STScnNode2dProps nodeProps);

/**
 * @brief Adds a model render commands into the commands container, using the provided node properties and mode, with the top model properties at the stack.
 * @param ref Reference to object.
 * @param model Model to add.
 * @param nodeProps Model properties to apply.
 * @param mode Node properties push mode.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRenderJob_model2dAddWithNodePropsAndMode(ScnRenderJobRef ref, ScnModel2dRef model, const STScnNode2dProps nodeProps, const ScnRenderJobPushMode mode);

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
