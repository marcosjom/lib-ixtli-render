//
//  ScnRenderFbuffState.h
//  ixtli-render
//
//  Created by Marcos Ortega on 22/8/25.
//

#ifndef ScnRenderFbuffState_h
#define ScnRenderFbuffState_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnAbsPtr.h"
#include "ixrender/core/ScnContext.h"
#include "ixrender/core/ScnArray.h"
#include "ixrender/gpu/ScnGpuFramebuffProps.h"
#include "ixrender/gpu/ScnGpuModelProps2d.h"
#include "ixrender/scene/ScnFramebuff.h"
#include "ixrender/scene/ScnVertexbuff.h"
#include "ixrender/scene/ScnTexture.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnRenderFbuffProps

/** @struct STScnRenderFbuffProps
 *  @brief Framebuffer drawing properties record in the commands stack. A new record is pushed into the stack when changing framebuffer or its properties. Shaders use these properties for scene calculation.
 *  @var STScnRenderFbuffProps::props
 *  Framebuffer drawing properties.
 *  @var STScnRenderFbuffProps::ptr
 *  Pointer in the scene properties buffer.
 */

#define STScnRenderFbuffProps_Zero   { STScnGpuFramebuffProps_Zero, STScnAbsPtr_Zero }

typedef struct STScnRenderFbuffProps {
    STScnGpuFramebuffProps  props;
    STScnAbsPtr             ptr;
} STScnRenderFbuffProps;

//STScnRenderFbuffState

/** @struct STScnRenderFbuffState
 *  @brief Framebuffer record in the commands stack. A new record is pushed into the stack when changing framebuffer. This record allows to determine when to push a framebuffer drawing properties in the stack.
 *  @var STScnRenderFbuffState::ctx
 *  Context.
 *  @var STScnRenderFbuffState::fbuff
 *  Framebuffer asociated to this record.
 *  @var STScnRenderFbuffState::stacks
 *  Properties and transforms stacks.
 *  @var STScnRenderFbuffState::active
 *  Last used vertexbuffer, texture and other objects; to determine when to push new cmds or ignore commands that do not produce real actions.
 */
typedef struct STScnRenderFbuffState {
    ScnContextRef       ctx;
    ScnFramebuffRef     fbuff;
    //stacks
    struct {
        ScnArrayStruct(props, STScnRenderFbuffProps);       //stack of props (viewport, ortho, ...)
        ScnArrayStruct(transforms, STScnGpuModelProps2d);   //stack of transformations (color, matrix, ...)
    } stacks;
    //active (last state, helps to reduce redundant cmds)
    struct {
        ScnVertexbuffRef vbuff;
        //texs
        struct {
            ScnTextureRef refs[ENScnGpuTextureIdx_Count];
            ScnUI32     changesSeq;
        } texs;
    } active;
} STScnRenderFbuffState;

/**
 * @brief Initializes the commands object.
 * @param ctx Context.
 * @param obj Reference to object.
 */
void ScnRenderFbuffState_init(ScnContextRef ctx, STScnRenderFbuffState* obj);

/**
 * @brief Destroys the commands object.
 * @param obj Reference to object.
 */
void ScnRenderFbuffState_destroy(STScnRenderFbuffState* obj);
//

/**
 * @brief Resets the commands activate state, forcing next commads to produce actions.
 * @param obj Reference to object.
 */
SC_INLN void ScnRenderFbuffState_resetActiveState(STScnRenderFbuffState* obj){
    ScnMemory_setZeroSt(obj->active);
}

/**
 * @brief Resets the commands object to its initialized state.
 * @param obj Reference to object.
 */
SC_INLN void ScnRenderFbuffState_reset(STScnRenderFbuffState* obj){
    ScnArray_empty(&obj->stacks.props);
    ScnArray_empty(&obj->stacks.transforms);
    ScnRenderFbuffState_resetActiveState(obj);
}

/**
 * @brief Pushes a framebuffer scene property into the stack.
 * @param obj Reference to object.
 * @param p Properties to push.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
SC_INLN ScnBOOL ScnRenderFbuffState_addProps(STScnRenderFbuffState* obj, const STScnRenderFbuffProps* const p){
    return ScnArray_addPtr(obj->ctx, &obj->stacks.props, p, STScnRenderFbuffProps) != NULL ? ScnTRUE : ScnFALSE;
}

/**
 * @brief Pushes a framebuffer scene transform into the stack.
 * @param obj Reference to object.
 * @param t Transorm to push.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
SC_INLN ScnBOOL ScnRenderFbuffState_addTransform(STScnRenderFbuffState* obj, const STScnGpuModelProps2d* const t){
    return ScnArray_addPtr(obj->ctx, &obj->stacks.transforms, t, STScnGpuModelProps2d) != NULL ? ScnTRUE : ScnFALSE;
}

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRenderFbuffState_h */
