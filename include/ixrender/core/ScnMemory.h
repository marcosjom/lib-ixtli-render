//
//  ScnMemory.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnMemory_h
#define ScnMemory_h

#include "ixrender/ixtli-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnMemoryItf

/** @struct STScnMemoryItf
 *  @brief Interface for memory allocation. This allows to link this library to your own management's core.
 *  @var STScnMemoryItf::malloc
 *  Function to allocate a memory chunck.
 *  @var STScnMemoryItf::realloc
 *  Function to resize a previously allocated memory chunk.
 *  @var STScnMemoryItf::free
 *  Function to free a previously allocated memory chunk.
 */

#define STScnMemoryItf_Zero { NULL, NULL, NULL }

typedef struct STScnMemoryItf {
    void*   (*malloc)(const ScnUI32 newSz, const char* dbgHintStr);
    void*   (*realloc)(void* ptr, const ScnUI32 newSz, const char* dbgHintStr);
    void    (*free)(void* ptr);
} STScnMemoryItf;

/**
 * @brief Links the provided interface's NULL methods to a DEFAULT implementation, this reduces the need to check for functions NULL pointers.
 * @note Passing a zero-intialized interface will provide the default implementation in return.
 * @param itf The interface to be populated
 */
void ScnMemoryItf_fillMissingMembers(STScnMemoryItf* itf);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnMemory_h */
