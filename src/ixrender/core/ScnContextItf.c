//
//  ScnContextItf.c
//  ixtli-render
//
//  Created by Marcos Ortega on 21/8/25.
//

#include "ixrender/core/ScnContextItf.h"

//STScnContextItf (API)

STScnContextItf ScnContextItf_getDefault(void){
    STScnContextItf itf;
    ScnMemory_setZeroSt(itf);
    ScnContextItf_fillMissingMembers(&itf);
    return itf;
}

//Links NULL methods to a DEFAULT implementation,
//this reduces the need to check for functions NULL pointers.
void ScnContextItf_fillMissingMembers(STScnContextItf* itf){
    if(itf == NULL) return;
    ScnMemoryItf_fillMissingMembers(&itf->mem);
    ScnMutexItf_fillMissingMembers(&itf->mutex);
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
