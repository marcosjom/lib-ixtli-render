//
//  ScnRenderCmds.h
//  ixtli-render
//
//  Created by Marcos Ortega on 10/8/25.
//

#ifndef ScnRenderCmds_h
#define ScnRenderCmds_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnArray.h"
#include "ixrender/core/ScnArraySorted.h"
#include "ixrender/core/ScnMemElastic.h"
#include "ixrender/gpu/ScnGpuDeviceDesc.h"
#include "ixrender/gpu/ScnGpuFramebuffProps.h"
#include "ixrender/gpu/ScnGpuModelProps2d.h"
#include "ixrender/scene/ScnFramebuff.h"
#include "ixrender/scene/ScnVertexbuff.h"
#include "ixrender/scene/ScnTexture.h"
#include "ixrender/scene/ScnRenderCmd.h"
#include "ixrender/scene/ScnRenderFbuffState.h"
#include "ixrender/scene/ScnRenderJobObj.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnRenderCmds

/** @struct STScnRenderCmds
 *  @brief Render job's commands container.
 *  @var STScnRenderCmds::ctx
 *  Context
 *  @var STScnRenderCmds::isPrepared
 *  Flag activated after a succesful call to prepare.
 *  @var STScnRenderCmds::gpuDevDesc
 *  Gpu device reference.
 *  @var STScnRenderCmds::mPropsScns
 *  Buffer's reference with scene properties.
 *  @var STScnRenderCmds::mPropsMdls
 *  Buffer's reference with models properties.
 *  @var STScnRenderCmds::stackUse
 *  Real usage of the framebuffers state stack. The stack slots are reused.
 *  @var STScnRenderCmds::stack
 *  Framebuffers state stack.
 *  @var STScnRenderCmds::cmds
 *  Render commands array.
 *  @var STScnRenderCmds::objs
 *  Map of objects used by the commands.
 */

typedef struct STScnRenderCmds {
    ScnContextRef       ctx;
    ScnBOOL             isPrepared;
    //
    STScnGpuDeviceDesc  gpuDevDesc;
    ScnMemElasticRef    mPropsScns;     //buffer with scenes properties (viewport, ortho-box)
    ScnMemElasticRef    mPropsMdls;     //buffer with models properties (color, matrices)
    //
    ScnSI32             stackUse;       //effective use of the stack-array (the array is reused between renders)
    ScnArrayStruct(     stack, STScnRenderFbuffState);   //stack of framebuffers
    ScnArrayStruct(     cmds, STScnRenderCmd);           //cmds sequence
    ScnArraySortedStruct(objs, STScnRenderJobObj);      //unique references of objects using for thias job
} STScnRenderCmds;

/**
 * @brief Initializes the commands container.
 * @param ctx Context.
 * @param obj Reference to object.
 */
void ScnRenderCmds_init(ScnContextRef ctx, STScnRenderCmds* obj);

/**
 * @brief Destroys the commands container.
 * @param obj Reference to object.
 */
void ScnRenderCmds_destroy(STScnRenderCmds* obj);

/**
 * @brief Prepares the commands container for adding commands.
 * @param obj Reference to object.
 * @param gpuDevDesc Reference to gpu device.
 * @param propsScnsPerBlock Ammount of scene properties per memory block in the elastic memory used for storing them.
 * @param propsMdlsPerBlock Ammount of models properties per memory block in the elastic memory used for storing them.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRenderCmds_prepare(STScnRenderCmds* obj, const STScnGpuDeviceDesc* gpuDevDesc, const ScnUI32 propsScnsPerBlock, const ScnUI32 propsMdlsPerBlock);

/**
 * @brief Resets a commands container to its prepared state.
 * @param obj Reference to object.
 */
void ScnRenderCmds_reset(STScnRenderCmds* obj);

/**
 * @brief Adds a command to the container.
 * @param obj Reference to object.
 * @param cmd Command to add.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRenderCmds_add(STScnRenderCmds* obj, const STScnRenderCmd* const cmd);

/**
 * @brief Adds an object to the container's map, as used by a command. Objects are added and retained once.
 * @param obj Reference to object.
 * @param type Object's type.
 * @param objRef Object's reference.
 * @param optDstIsFirstUse Destination to flag that determines if the object was added in this call or if already was in the map; optional.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnRenderCmds_addUsedObj(STScnRenderCmds* obj, const ENScnRenderJobObjType type, ScnObjRef* objRef, ScnBOOL* optDstIsFirstUse);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRenderCmds_h */
