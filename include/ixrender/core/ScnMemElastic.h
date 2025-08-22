//
//  ScnMemElastic.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnMemElastic_h
#define ScnMemElastic_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/core/ScnMemBlock.h"
#include "ixrender/core/ScnMemElasticCfg.h"
#include "ixrender/type/ScnRange.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnMemElasticRef

/** @struct ScnMemElasticRef
 *  @brief ScnMemElastic shared pointer. This represents a chain of fixed-size memory blocks that can assign portions of themselves.
 *  @note The chain is automatically expanded or contracted by adding or removing blocks at its end. Previously assigned addreses are still valid after the chain grows or shrinks.
 *  @note Each memory block could be a system memory chunk or another abstract object, like a gpu-heap.
 *  @note All returned indices are relative to the whole chain.
 */

#define ScnMemElasticRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnMemElastic)

//

/**
 * @brief Prepares the chain with the provided configuration.
 * @param ref Reference to object.
 * @param cfg Chain configuration.
 * @param optItf Allocator interface, optional.
 * @param optItfParam Allocator interface parameter, optional.
 * @param optDstBlocksTotalSz Destination for the total assignable memory size in bytes, optional. Bigger size than the specified in the config could be allocated.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnMemElastic_prepare(ScnMemElasticRef ref, const STScnMemElasticCfg* cfg, STScnMemBlockAllocItf* optItf, void* optItfParam, ScnUI32* optDstBlocksTotalSz);

/**
 * @brief Determines if the chain has pointers allocated.
 * @param ref Reference to object.
 * @return ScnTRUE if has pointers allocated, ScnFALSE otherwise.
 */
ScnBOOL ScnMemElastic_hasPtrs(ScnMemElasticRef ref); //allocations made?

/**
 * @brief Retrieves the addressable size of the chain.
 * @param ref Reference to object.
 * @return The size of the chain in bytes, including the address zero.
 */
ScnUI32     ScnMemElastic_getAddressableSize(ScnMemElasticRef ref); //includes the address zero

/**
 * @brief Retrieves the range that encloses the first and last address allocated.
 * @param ref Reference to object.
 * @return The range that includes first and last address allocated.
 */
STScnRangeU ScnMemElastic_getUsedAddressesRng(ScnMemElasticRef ref); //range that covers all allocated addresses index

/**
 * @brief Retrieves the range that encloses the first and last address allocated, in memory-size-alignment provided in the configuration at prepare.
 * @note This is useful for memory operations that require respecting an alignment, like copying memory from cpu to gpu.
 * @param ref Reference to object.
 * @return The range that includes first and last address allocated.
 */
STScnRangeU ScnMemElastic_getUsedAddressesRngAligned(ScnMemElasticRef ref); //range that covers all allocated addresses index

/**
 * @brief Retrieves the pointer or the next continuous memory chunk at the indexed position provided.
 * @note This is used to read the chain internal memory blocks.
 * @param ref Reference to object.
 * @param iAddress Address index.
 * @param optDstContinuousSz Destination of the size of the continuous address space, optional.
 * @return The pointer to the next continuous address, or STScnAbsPtr_Zero if the index is beyond of the chain's size.
 */
STScnAbsPtr ScnMemElastic_getNextContinuousAddress(ScnMemElasticRef ref, const ScnUI32 iAddress, ScnUI32* optDstContinuousSz);


//allocations

/**
 * @brief Allocates an internal range of the memory chain.
 * @param ref Reference to object.
 * @param usableSz Size of the required range.
 * @param optDstBlocksTotalSz Destination of the size of the chain after the assign, to determine if a new block was added to the chain.
 * @return The address allocated inside the chunk.
 */
STScnAbsPtr ScnMemElastic_malloc(ScnMemElasticRef ref, const ScnUI32 usableSz, ScnUI32* optDstBlocksTotalSz);

/**
 * @brief Frees an internal range of the memory block.
 * @param ref Reference to object.
 * @param ptr A previously allocated internal pointer.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnMemElastic_mfree(ScnMemElasticRef ref, const STScnAbsPtr ptr);

/**
 * @brief Converts a previously returned pointer from chain-address to it's block-address.
 * @param ref Reference to object.
 * @param ptr A previously allocated internal pointer.
 * @return The address realtive to its internal block, or STScnAbsPtr_Zero if the index is beyond of the chain's size.
 */
STScnAbsPtr ScnMemElastic_mPtrToBlockPtr(ScnMemElasticRef ref, const STScnAbsPtr ptr);

/**
 * @brief Removes all the allocated-pointers from the index, and flags the whole chain as available for allocation.
 * @note All previously allocated pointers becomes invalid and should no be used.
 * @param ref Reference to object.
 */
void ScnMemElastic_clear(ScnMemElasticRef ref);


//dbg

/**
 * @brief Sends the allocation-index to the specified interface.
 * @note Intended for debug purposes only.
 * @param ref Reference to object.
 * @param fnc Method to pass the pointers to.
 * @param fncParam Param to pass the method.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnMemElastic_pushPtrs(ScnMemElasticRef ref, ScnMemBlockPushBlockPtrsFunc fnc, void* fncParam);

/**
 * @brief Validates the allocated-pointers index, gaps and internal variables. All gaps and pointers should be sequential.
 * @note Intended for debug purposes only.
 * @param ref Reference to object.
 * @return ScnTRUE if valid, ScnFALSE otherwise.
 */
ScnBOOL ScnMemElastic_validateIndex(ScnMemElasticRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnMemElastic_h */
