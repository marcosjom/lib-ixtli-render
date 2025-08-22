//
//  ScnGpuBuffer.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnGpuBuffer_h
#define ScnGpuBuffer_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/core/ScnMemElastic.h"
#include "ixrender/type/ScnRange.h"
#include "ixrender/scene/ScnVertex.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnGpuBufferCfg

/** @struct STScnGpuBufferCfg
 *  @brief Gpu buffer configuration.
 *  @var STScnGpuBufferCfg::mem
 *  Buffer's memory configuration.
 */

#define STScnGpuBufferCfg_Zero   { STScnMemElasticCfg_Zero }

typedef struct STScnGpuBufferCfg {
    STScnMemElasticCfg  mem;    //memory blocks cfg
} STScnGpuBufferCfg;

//STScnGpuBufferChanges

/** @struct STScnGpuBufferChanges
 *  @brief Gpu buffer ranges that require synchronization.
 *  @var STScnGpuBufferChanges::all
 *  The whole buffer must be synchronized.
 *  @var STScnGpuBufferChanges::rngs
 *  Ranges that require synchronization.
 *  @var STScnGpuBufferChanges::rngsUse
 *  Ammount of ranges that require synchronization.
 */

#define STScnGpuBufferChanges_Zero  { ScnFALSE, NULL, 0 }
#define STScnGpuBufferChanges_All   { ScnTRUE, NULL, 0 }

typedef struct STScnGpuBufferChanges {
    ScnBOOL         all;    //the whoe buffer requires synchronization
    STScnRangeU*    rngs;   //rngs changed
    ScnUI32         rngsUse;
} STScnGpuBufferChanges;

//STScnGpuBufferApiItf

/** @struct STScnGpuBufferApiItf
 *  @brief Gpu buffer interface.
 *  @var STScnGpuBufferApiItf::free
 *  Method to free the buffer.
 *  @var STScnGpuBufferApiItf::sync
 *  Method to synchronizr the buffer's data.
 */

typedef struct STScnGpuBufferApiItf {
    void    (*free)(void* data);
    ScnBOOL (*sync)(void* data, ScnMemElasticRef mem, const STScnGpuBufferChanges* changes);
} STScnGpuBufferApiItf;

//ScnGpuBufferRef

/** @struct ScnGpuBufferRef
 *  @brief ScnGpuBuffer shared pointer. An abstract object based on the currently used API.
 */

#define ScnGpuBufferRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnGpuBuffer)

//

/**
 * @brief Prepares the gpu abstract object with the provided interface.
 * @param ref Reference to object.
 * @param itf Interface to the API.
 * @param itfParam Parameter to be given to the interface's  methods.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnGpuBuffer_prepare(ScnGpuBufferRef ref, const STScnGpuBufferApiItf* itf, void* itfParam);

/**
 * @brief Retrieves the gpu abstract object pointer.
 * @param ref Reference to object.
 * @return Abstract object's pointer on success, NULL otherwise.
 */
void* ScnGpuBuffer_getApiItfParam(ScnGpuBufferRef ref);

/**
 * @brief Synchronizes the gpu buffer's data with the provided memory and changes.
 * @note The provided memory should be able to be modified after calling this method without affecting the ongoing syncronization.
 * @param ref Reference to object.
 * @param mem Memory object.
 * @param changes Memory changes since last synchronization.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnGpuBuffer_sync(ScnGpuBufferRef ref, ScnMemElasticRef mem, const STScnGpuBufferChanges* changes);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuBuffer_h */
