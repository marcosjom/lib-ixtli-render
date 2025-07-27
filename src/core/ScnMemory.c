//
//  ScnMemory.c
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/core/ScnMemory.h"
#include <stdlib.h> //malloc, realloc, free

//STScnMemoryItf (API)

void* ScnMemoryItf_default_malloc(const ScnUI32 newSz, const char* dbgHintStr){
    return malloc(newSz);
}

void* ScnMemoryItf_default_realloc(void* ptr, const ScnUI32 newSz, const char* dbgHintStr){
    //"If there is not enough memory, the old memory block is not freed and null pointer is returned."
    return realloc(ptr, newSz);
}

void ScnMemoryItf_default_free(void* ptr){
    free(ptr);
}

//Links NULL methods to a DEFAULT implementation,
//this reduces the need to check for functions NULL pointers.
void ScnMemoryItf_fillMissingMembers(STScnMemoryItf* itf){
    if(itf == NULL) return;
    SCN_ITF_SET_MISSING_METHOD_TO_DEFAULT(itf, ScnMemoryItf, malloc);
    SCN_ITF_SET_MISSING_METHOD_TO_DEFAULT(itf, ScnMemoryItf, realloc);
    SCN_ITF_SET_MISSING_METHOD_TO_DEFAULT(itf, ScnMemoryItf, free);
    //validate missing implementations
#   ifdef SCN_ASSERTS_ACTIVATED
    {
        void** ptr = (void**)itf;
        void** afterEnd = ptr + (sizeof(*itf) / sizeof(void*));
        while(ptr < afterEnd){
            SCN_ASSERT(*ptr != NULL) //function pointer should be defined
            ptr++;
        }
    }
#   endif
}
