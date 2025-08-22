//
//  ScnModel2d.h
//  ixtli-render
//
//  Created by Marcos Ortega on 28/7/25.
//

#ifndef ScnModel2d_h
#define ScnModel2d_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/type/ScnColor.h"
#include "ixrender/type/ScnPoint.h"
#include "ixrender/type/ScnSize.h"
#include "ixrender/scene/ScnTexture.h"
#include "ixrender/scene/ScnTransform2d.h"
#include "ixrender/scene/ScnModel2dCmd.h"
#include "ixrender/scene/ScnNode2dProps.h"
#include "ixrender/scene/ScnRenderCmd.h"
#include "ixrender/scene/ScnVertexbuffs.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnModel2dPushCmdsFunc

/** @typedef ScnModel2dPushCmdsFunc
 *  @brief Function to send a model commands to.
 */

typedef ScnBOOL (*ScnModel2dPushCmdsFunc)(void* data, const STScnGpuModelProps2d* const props, const STScnModel2dCmd* const cmds, const ScnUI32 cmdsSz);

//ScnModel2dRef

/** @struct ScnModel2dRef
 *  @brief ScnModel2d shared pointer. An object that contains the render commands for a model, including its vertices and indices.
 */

#define ScnModel2dRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnModel2d)

//

/**
 * @brief Sets the model's vertebuffers source for vertices and indices.
 * @param ref Reference to object.
 * @param vbuffs Reference to vertebuffers.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnModel2d_setVertexBuffs(ScnModel2dRef ref, ScnVertexbuffsRef vbuffs);

//draw commands

/**
 * @brief Removes all draw commands and releasing their vertices, indices, textures and other resources.
 * @param ref Reference to object.
 */
void ScnModel2d_resetDrawCmds(ScnModel2dRef ref);

/**
 * @brief Adds a draw command with zero textures vertices.
 * @note The pointers returned by this method are valid until the model is released or reseted.
 * @param ref Reference to object.
 * @param shape Drawing shape.
 * @param count Ammount of vertices to allocate.
 * @return Pointer to the allocated vertices on success, STScnVertex2DPtr_Zero otherwise.
 */
STScnVertex2DPtr ScnModel2d_addDraw(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 count);

/**
 * @brief Adds a draw command with one texture vertices.
 * @note The pointers returned by this method are valid until the model is released or reseted.
 * @param ref Reference to object.
 * @param shape Drawing shape.
 * @param count Ammount of vertices to allocate.
 * @return Pointer to the allocated vertices on success, STScnVertex2DTexPtr_Zero otherwise.
 */
STScnVertex2DTexPtr ScnModel2d_addDrawTex(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnTextureRef t0);

/**
 * @brief Adds a draw command with two textures vertices.
 * @note The pointers returned by this method are valid until the model is released or reseted.
 * @param ref Reference to object.
 * @param shape Drawing shape.
 * @param count Ammount of vertices to allocate.
 * @return Pointer to the allocated vertices on success, STScnVertex2DTex2Ptr_Zero otherwise.
 */
STScnVertex2DTex2Ptr ScnModel2d_addDrawTex2(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnTextureRef t0, ScnTextureRef t1);

/**
 * @brief Adds a draw command with three textures vertices.
 * @note The pointers returned by this method are valid until the model is released or reseted.
 * @param ref Reference to object.
 * @param shape Drawing shape.
 * @param count Ammount of vertices to allocate.
 * @return Pointer to the allocated vertices on success, STScnVertex2DTex3Ptr_Zero otherwise.
 */
STScnVertex2DTex3Ptr ScnModel2d_addDrawTex3(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnTextureRef t0, ScnTextureRef t1, ScnTextureRef t2);

/**
 * @brief Flags the previously allocated vertices for synchronization to gpu-buffers. Call this after updating the vertices of a draw command.
 * @param ref Reference to object.
 * @param ptr Pointer to vertices.
 * @param count Ammount of vertices to flag.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnModel2d_v0FlagForSync(ScnModel2dRef ref, STScnVertex2DPtr ptr, const ScnUI32 count);

/**
 * @brief Flags the previously allocated vertices for synchronization to gpu-buffers. Call this after updating the vertices of a draw command.
 * @param ref Reference to object.
 * @param ptr Pointer to vertices.
 * @param count Ammount of vertices to flag.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnModel2d_v1FlagForSync(ScnModel2dRef ref, STScnVertex2DTexPtr ptr, const ScnUI32 count);

/**
 * @brief Flags the previously allocated vertices for synchronization to gpu-buffers. Call this after updating the vertices of a draw command.
 * @param ref Reference to object.
 * @param ptr Pointer to vertices.
 * @param count Ammount of vertices to flag.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnModel2d_v2FlagForSync(ScnModel2dRef ref, STScnVertex2DTex2Ptr ptr, const ScnUI32 count);

/**
 * @brief Flags the previously allocated vertices for synchronization to gpu-buffers. Call this after updating the vertices of a draw command.
 * @param ref Reference to object.
 * @param ptr Pointer to vertices.
 * @param count Ammount of vertices to flag.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnModel2d_v3FlagForSync(ScnModel2dRef ref, STScnVertex2DTex3Ptr ptr, const ScnUI32 count);

/**
 * @brief Updates the texture linked to a drawing command, the texture at index-0.
 * @param ref Reference to object.
 * @param ptr Pointer to vertices of the draw command.
 * @param tex Texture to be linked.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnModel2d_v1UpdateTexture(ScnModel2dRef ref, STScnVertex2DTexPtr ptr, ScnTextureRef tex);

/**
 * @brief Updates the texture linked to a drawing command at the specified index.
 * @param ref Reference to object.
 * @param ptr Pointer to vertices of the draw command.
 * @param tex Texture to be linked.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnModel2d_v2UpdateTexture(ScnModel2dRef ref, STScnVertex2DTex2Ptr ptr, ScnTextureRef tex, const ScnUI32 iTex);

/**
 * @brief Updates the texture linked to a drawing command at the specified index.
 * @param ref Reference to object.
 * @param ptr Pointer to vertices of the draw command.
 * @param tex Texture to be linked.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnModel2d_v3UpdateTexture(ScnModel2dRef ref, STScnVertex2DTex3Ptr ptr, ScnTextureRef tex, const ScnUI32 iTex);

/**
 * @brief Adds a draw command with indices pointing to zero texture vertices.
 * @note The pointers returned by this method are valid until the model is released or reseted.
 * @param ref Reference to object.
 * @param shape Drawing shape.
 * @param countIdxs Ammount of indices to allocate.
 * @param countVerts Ammount of vertices to allocate.
 * @param dstVerts Destination for the allocated vertices pointer.
 * @return Pointer to the allocated indices on success, STScnVertexIdxPtr_Zero otherwise.
 */
STScnVertexIdxPtr ScnModel2d_addDrawIndexed(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, const ScnUI32 countVerts, STScnVertex2DPtr* dstVerts);

/**
 * @brief Adds a draw command with indices pointing to one texture vertices.
 * @note The pointers returned by this method are valid until the model is released or reseted.
 * @param ref Reference to object.
 * @param shape Drawing shape.
 * @param countIdxs Ammount of indices to allocate.
 * @param t0 Texture index-0 to link.
 * @param countVerts Ammount of vertices to allocate.
 * @param dstVerts Destination for the allocated vertices pointer.
 * @return Pointer to the allocated indices on success, STScnVertexIdxPtr_Zero otherwise.
 */
STScnVertexIdxPtr ScnModel2d_addDrawIndexedTex(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnTextureRef t0, const ScnUI32 countVerts, STScnVertex2DTexPtr* dstVerts);

/**
 * @brief Adds a draw command with indices pointing to two textures vertices.
 * @note The pointers returned by this method are valid until the model is released or reseted.
 * @param ref Reference to object.
 * @param shape Drawing shape.
 * @param countIdxs Ammount of indices to allocate.
 * @param t0 Texture index-0 to link.
 * @param t1 Texture index-1 to link.
 * @param countVerts Ammount of vertices to allocate.
 * @param dstVerts Destination for the allocated vertices pointer.
 * @return Pointer to the allocated indices on success, STScnVertexIdxPtr_Zero otherwise.
 */
STScnVertexIdxPtr ScnModel2d_addDrawIndexedTex2(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnTextureRef t0, ScnTextureRef t1, const ScnUI32 countVerts, STScnVertex2DTex2Ptr* dstVerts);

/**
 * @brief Adds a draw command with indices pointing to three textures vertices.
 * @note The pointers returned by this method are valid until the model is released or reseted.
 * @param ref Reference to object.
 * @param shape Drawing shape.
 * @param countIdxs Ammount of indices to allocate.
 * @param t0 Texture index-0 to link.
 * @param t1 Texture index-1 to link.
 * @param t2 Texture index-2 to link.
 * @param countVerts Ammount of vertices to allocate.
 * @param dstVerts Destination for the allocated vertices pointer.
 * @return Pointer to the allocated indices on success, STScnVertexIdxPtr_Zero otherwise.
 */
STScnVertexIdxPtr ScnModel2d_addDrawIndexedTex3(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnTextureRef t0, ScnTextureRef t1, ScnTextureRef t2, const ScnUI32 countVerts, STScnVertex2DTex3Ptr* dstVerts);

/**
 * @brief Flags the previously allocated zero texture indices for synchronization to gpu-buffers. Call this after updating the vertices of a draw command.
 * @param ref Reference to object.
 * @param ptr Pointer to indices.
 * @param count Ammount of indices to flag.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnModel2d_i0FlagForSync(ScnModel2dRef ref, STScnVertexIdxPtr ptr, const ScnUI32 count);

/**
 * @brief Flags the previously allocated one texture indices for synchronization to gpu-buffers. Call this after updating the vertices of a draw command.
 * @param ref Reference to object.
 * @param ptr Pointer to indices.
 * @param count Ammount of indices to flag.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnModel2d_i1FlagForSync(ScnModel2dRef ref, STScnVertexIdxPtr ptr, const ScnUI32 count);

/**
 * @brief Flags the previously allocated two textures indices for synchronization to gpu-buffers. Call this after updating the vertices of a draw command.
 * @param ref Reference to object.
 * @param ptr Pointer to indices.
 * @param count Ammount of indices to flag.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnModel2d_i2FlagForSync(ScnModel2dRef ref, STScnVertexIdxPtr ptr, const ScnUI32 count);

/**
 * @brief Flags the previously allocated three textures indices for synchronization to gpu-buffers. Call this after updating the vertices of a draw command.
 * @param ref Reference to object.
 * @param ptr Pointer to indices.
 * @param count Ammount of indices to flag.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnModel2d_i3FlagForSync(ScnModel2dRef ref, STScnVertexIdxPtr ptr, const ScnUI32 count);

/**
 * @brief Updates the texture linked to a one texture indexed drawing command at index-0.
 * @param ref Reference to object.
 * @param ptr Pointer to vertices of the draw command.
 * @param tex Texture to be linked.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnModel2d_i1UpdateTexture(ScnModel2dRef ref, STScnVertexIdxPtr ptr, ScnTextureRef tex);

/**
 * @brief Updates the texture linked to a two textures indexed drawing command at the specified index.
 * @param ref Reference to object.
 * @param ptr Pointer to vertices of the draw command.
 * @param tex Texture to be linked.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnModel2d_i2UpdateTexture(ScnModel2dRef ref, STScnVertexIdxPtr ptr, ScnTextureRef tex, const ScnUI32 iTex);

/**
 * @brief Updates the texture linked to a three textures  indexed drawing command at the specified index.
 * @param ref Reference to object.
 * @param ptr Pointer to vertices of the draw command.
 * @param tex Texture to be linked.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnModel2d_i3UpdateTexture(ScnModel2dRef ref, STScnVertexIdxPtr ptr, ScnTextureRef tex, const ScnUI32 iTex);

//draw commands to consumer

/**
 * @brief Pushes the commands forward to the specified method. This is the method used for temporary exposing the internal commands.
 * @param ref Reference to object.
 * @param props Model properties to pass forward to the method.
 * @param fnc Method to use forward.
 * @param fncParam Opaque parameter to pass forward.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnModel2d_sendRenderCmds(ScnModel2dRef ref, const STScnGpuModelProps2d* const props, ScnModel2dPushCmdsFunc fnc, void* fncParam);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnModel2d_h */
