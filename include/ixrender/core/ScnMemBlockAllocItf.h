//
//  ScnMemBlockAllocItf.h
//  ixtli-render
//
//  Created by Marcos Ortega on 21/8/25.
//

#ifndef ScnMemBlockAllocItf_h
#define ScnMemBlockAllocItf_h

#include "ixrender/ixtli-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnMemBlockAllocItf_h */
