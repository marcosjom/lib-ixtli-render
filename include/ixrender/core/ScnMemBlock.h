//
//  ScnMemBlock.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnMemBlock_h
#define ScnMemBlock_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/type/ScnRange.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnMemBlockPtr

/** @struct STScnMemBlockPtr
 *  @brief Describes a block of allocated memory.
 *  @var STScnMemBlockPtr::ptr
 *  Pointer to memory.
 *  @var STScnMemBlockPtr::sz
 *  Size of memory block.
 */

#define STScnMemBlockPtr_Zero { NULL, 0 }

typedef struct STScnMemBlockPtr {
    void*       ptr;  //pointer returned by 'ScnMemBlock_malloc'
    ScnUI32     sz;   //size at 'ScnMemBlock_malloc' call
} STScnMemBlockPtr;

//STScnAbsPtr, abstract pointer

/** @struct STScnAbsPtr
 *  @brief Describes a reserved memory chunk.
 *  @var STScnAbsPtr::ptr
 *  Pointer to memory. This pointer is an absolute address and should be valid until freed.
 *  @var STScnAbsPtr::idx
 *  Relative index; index zero is the first address.
 *  @var STScnAbsPtr::itfParam
 *  Opaque object to which this chunk belongs.
 */

#define STScnAbsPtr_Zero { NULL, 0, NULL }

typedef struct STScnAbsPtr {
    void*       ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    ScnUI32     idx;    //abstract address
    void*       itfParam; //object to wich the memory block belongs (the ScnMemBlock itself if no custom itf was provided)
} STScnAbsPtr;

//STScnMemBlockCfg

/** @struct STScnMemBlockCfg
 *  @brief Memory block configuration.
 *  @var STScnMemBlockCfg::size
 *  Size of the block in bytes; the size never changes once the block is allocated.
 *  @var STScnMemBlockCfg::sizeAlign
 *  Alignment of the block's size. A block bigger that the requested size could be allocated to respect this alignment.
 *  @var STScnMemBlockCfg::idxsAlign
 *  Block's internal allocations aligment. A chunk bigger than the requjest size could be allocated to respect this alignment.
 *  @var STScnMemBlockCfg::isIdxZeroValid
 *  Determines if the idx==0 is a valid address for allocations; if not, idx=idxsAlign is the first address asignable.
 */

#define STScnMemBlockCfg_Zero { 0, 0, 0, ScnFALSE }

typedef struct STScnMemBlockCfg {
    ScnUI32 size;           //ammount of bytes allocable (including the idx-0)
    ScnUI32 sizeAlign;      //whole memory block size alignment
    ScnUI32 idxsAlign;      //individual pointers alignment
    ScnBOOL isIdxZeroValid; //idx=0 is an assignable address, if not, the first assignable address is 'idxsAlign * 1'.
} STScnMemBlockCfg;

//STScnMemBlockItf

/** @struct STScnMemBlockItf
 *  @brief Interface for allocation of the ScnMemBlock internal fixed memory chunk.
 *  @note The memory chunk could be a system memory chunk or another abstract object, like a gpu-heap.
 *  @var STScnMemBlockItf::malloc
 *  Method for allocating the memory chunk.
 *  @var STScnMemBlockItf::free
 *  Method for freeing the memory chunk.
 */

#define STScnMemBlockAllocItf_Zero  { NULL, NULL }

typedef struct STScnMemBlockAllocItf {
    void*   (*malloc)(const ScnUI32 size, const char* dbgHintStr, void* itfParam);
    void    (*free)(void* ptr, void* itfParam);
} STScnMemBlockAllocItf;

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

//STScnMemPushPtrsItf

/** @struct STScnMemPushPtrsItf
 *  @brief Interface to pass forward the pointers allocated inside a ScnMemChunk.
 *  @var STScnMemPushPtrsItf::pushBlockPtrs
 *  Method to which the allocated pointers will be pushed to.
 */

#define STScnMemPushPtrsItf_Zero  { NULL }

typedef struct STScnMemPushPtrsItf {
    ScnBOOL (*pushBlockPtrs)(void* data, const ScnUI32 rootIndex, const void* rootAddress, const STScnMemBlockPtr* const ptrs, const ScnUI32 ptrsSz);
} STScnMemPushPtrsItf;

/**
 * @brief Sends the allocation-index to the specified interface.
 * @note Intended for debug purposes only.
 * @param ref Reference to object.
 * @param rootIndexToPass Index/offset value to be passed down to the interface.
 * @param itf Interface to pass the pointers to.
 * @param itfParam Param to pass the interface methods.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnMemBlock_pushPtrs(ScnMemBlockRef ref, const ScnUI32 rootIndexToPass, STScnMemPushPtrsItf* itf, void* itfParam);

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
