//
//  ScnMemBlock.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnMemBlock_h
#define ScnMemBlock_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnAbsPtr.h"
#include "ixrender/core/ScnMemBlockPtr.h"
#include "ixrender/core/ScnMemBlockAllocItf.h"
#include "ixrender/core/ScnMemBlockCfg.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/type/ScnRange.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnMemBlockRef

/** @struct ScnMemBlockRef
 *  @brief ScnMemBlock shared pointer. This represents a fixed-size memory block that can asign portions of itself.
 *  @note The memory chunk could be a system memory chunk or another abstract object, like a gpu-heap.
 *  @note It contains an index of all internal allocated pointers and a map of all ranges (gaps) available for allocation.
 */

#define ScnMemBlockRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnMemBlock)

/**
 * @brief Allocated the fixed-size memory block.
 * @param ref Reference to object.
 * @param cfg Block's configuration.
 * @param optItf Block's allocator interface, optional. If null, the ScnContext becomes the allocator.
 * @param optItfParam Block's allocator interface param. optional.
 * @param optDstPtrAfterEnd Pointer to the next address after the block's memory chunk. This can be used to determine the real size of the allocated chunk/block.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnMemBlock_prepare(ScnMemBlockRef ref, const STScnMemBlockCfg* cfg, STScnMemBlockAllocItf* optItf, void* optItfParam, STScnAbsPtr* optDstPtrAfterEnd);

/**
 * @brief Determines if the block has pointers allocated.
 * @param ref Reference to object.
 * @return ScnTRUE if has pointers allocated, ScnFALSE otherwise.
 */
ScnBOOL ScnMemBlock_hasPtrs(ScnMemBlockRef ref); //allocations made?

/**
 * @brief Retrieves the addressable size of the chunk.
 * @param ref Reference to object.
 * @return The size of the chunk in bytes, including the address zero.
 */
ScnUI32 ScnMemBlock_getAddressableSize(ScnMemBlockRef ref); //includes the address zero

/**
 * @brief Retrieves the first address of the chunk.
 * @param ref Reference to object.
 * @return The first address of the chunk.
 */
STScnAbsPtr ScnMemBlock_getStarAddress(ScnMemBlockRef ref); //includes the address zero

/**
 * @brief Retrieves the range that encloses the first and last address allocated.
 * @param ref Reference to object.
 * @return The range that includes first and last address allocated.
 */
STScnRangeU ScnMemBlock_getUsedAddressesRng(ScnMemBlockRef ref); //range that covers all allocated addresses index

//allocations

/**
 * @brief Allocates an internal range of the memory block.
 * @param ref Reference to object.
 * @param usableSz Size of the required range.
 * @return The address allocated inside the chunk.
 */
STScnAbsPtr ScnMemBlock_malloc(ScnMemBlockRef ref, const ScnUI32 usableSz);

/**
 * @brief Frees an internal range of the memory block.
 * @param ref Reference to object.
 * @param ptr A previously allocated internal pointer.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnMemBlock_mfree(ScnMemBlockRef ref, const STScnAbsPtr ptr);

/**
 * @brief Retrieves the sum of the gaps available for allocation.
 * @param ref Reference to object.
 * @return The ammount of bytes available for allocation.
 */
ScnUI32 ScnMemBlock_mAvailSz(ScnMemBlockRef ref);

/**
 * @brief If necesary, increases the size of the allocated-pointers index, in preparation for the specified future allocation.
 * @param ref Reference to object.
 * @param ammActions Ammount of future allocations expected.
 */
void ScnMemBlock_prepareForNewMallocsActions(ScnMemBlockRef ref, const ScnUI32 ammActions);   //increases the index's sz

/**
 * @brief Removes all the allocated-pointers from the index, and flags the whole block as a gap available for allocation.
 * @note All previously allocated pointers becomes invalid and should no be used.
 * @param ref Reference to object.
 */
void ScnMemBlock_clear(ScnMemBlockRef ref); //clears the index, all pointers are invalid after this call

//dbg

//ScnMemBlockPushBlockPtrsFunc

/** @typedef ScnMemBlockPushBlockPtrsFunc
 *  @brief Function to send a allocated pointers o.
 */

typedef ScnBOOL (*ScnMemBlockPushBlockPtrsFunc)(void* data, const ScnUI32 rootIndex, const void* rootAddress, const STScnMemBlockPtr* const ptrs, const ScnUI32 ptrsSz);

/**
 * @brief Sends the allocation-index to the specified interface.
 * @note Intended for debug purposes only.
 * @param ref Reference to object.
 * @param rootIndexToPass Index/offset value to be passed down to the interface.
 * @param fnc Method to pass the pointers to.
 * @param fncParam Param to pass to the method.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnMemBlock_pushPtrs(ScnMemBlockRef ref, const ScnUI32 rootIndexToPass, ScnMemBlockPushBlockPtrsFunc fnc, void* fncParam);

/**
 * @brief Validates the allocated-pointers index, gaps and internal variables. All gaps and pointers should be sequential.
 * @note Intended for debug purposes only.
 * @param ref Reference to object.
 * @return ScnTRUE if valid, ScnFALSE otherwise.
 */
ScnBOOL ScnMemBlock_validateIndex(ScnMemBlockRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnMemBlock_h */
