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

typedef struct STScnMemoryItf {
    void*   (*malloc)(const ScnUI32 newSz, const char* dbgHintStr);
    void*   (*realloc)(void* ptr, const ScnUI32 newSz, const char* dbgHintStr);
    void    (*free)(void* ptr);
} STScnMemoryItf;

// Links NULL methods to a DEFAULT implementation,
// this reduces the need to check for functions NULL pointers.
void ScnMemoryItf_fillMissingMembers(STScnMemoryItf* itf);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnMemory_h */
