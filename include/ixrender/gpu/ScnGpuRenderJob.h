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

/** @struct STScnGpuRenderJobApiItf
 *  @brief Render job's API interface.
 *  @var STScnGpuRenderJobApiItf::free
 *  Method to free the render job.
 *  @var STScnGpuRenderJobApiItf::getState
 *  Method to retrieve the state of the job.
 *  @var STScnGpuRenderJobApiItf::buildBegin
 *  Method to start to build the job.
 *  @var STScnGpuRenderJobApiItf::buildAddCmds
 *  Method to add commands to the job.
 *  @var STScnGpuRenderJobApiItf::buildEndAndEnqueue
 *  Method to finalize the job and send it to the gpu.
 */
typedef struct STScnGpuRenderJobApiItf {
    void                    (*free)(void* data);
    ENScnGpuRenderJobState  (*getState)(void* data);
    ScnBOOL                 (*buildBegin)(void* data, ScnGpuBufferRef bPropsScns, ScnGpuBufferRef bPropsMdls);
    ScnBOOL                 (*buildAddCmds)(void* data, const struct STScnRenderCmd* const cmds, const ScnUI32 cmdsSz);
    ScnBOOL                 (*buildEndAndEnqueue)(void* data);
} STScnGpuRenderJobApiItf;


//ScnGpuRenderJobRef

/** @struct ScnGpuRenderJobRef
 *  @brief ScnGpuRenderJob shared pointer. An abstract object based on the currently used API.
 */

#define ScnGpuRenderJobRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnGpuRenderJob)

//

/**
 * @brief Prepares the gpu abstract object with the provided interface.
 * @param ref Reference to object.
 * @param itf Interface to the API.
 * @param itfParam Parameter to be given to the interface's  methods.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnGpuRenderJob_prepare(ScnGpuRenderJobRef ref, const STScnGpuRenderJobApiItf* itf, void* itfParam);

/**
 * @brief Retrieves the gpu abstract object pointer.
 * @param ref Reference to object.
 * @return Abstract object's pointer on success, NULL otherwise.
 */
void* ScnGpuRenderJob_getApiItfParam(ScnGpuRenderJobRef ref);

/**
 * @brief Retrieves the render job's state.
 * @param ref Reference to object.
 * @return Job's state on success, ENScnGpuRenderJobState_Unknown otherwise.
 */
ENScnGpuRenderJobState ScnGpuRenderJob_getState(ScnGpuRenderJobRef ref);

/**
 * @brief Prepares the job's state for incoming commands.
 * @param ref Reference to object.
 * @param bPropsScns Buffer with scenes render properties.
 * @param bPropsMdls Buffer with models render properties.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnGpuRenderJob_buildBegin(ScnGpuRenderJobRef ref, ScnGpuBufferRef bPropsScns, ScnGpuBufferRef bPropsMdls);

/**
 * @brief Adds commands to the render job.
 * @param ref Reference to object.
 * @param cmds Commands to add.
 * @param cmdsSz Ammount of commands.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnGpuRenderJob_buildAddCmds(ScnGpuRenderJobRef ref, const struct STScnRenderCmd* const cmds, const ScnUI32 cmdsSz);

/**
 * @brief Finalizes the job and sends it to the gpu.
 * @param ref Reference to object.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnGpuRenderJob_buildEndAndEnqueue(ScnGpuRenderJobRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuRenderJob_h */
