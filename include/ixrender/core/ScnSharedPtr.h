//
//  ScnSharedPtr.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnSharedPtr_h
#define ScnSharedPtr_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnMemory.h"
#include "ixrender/core/ScnMutex.h"

#ifdef __cplusplus
extern "C" {
#endif

struct STScnSharedPtr_;
//
struct STScnContextItf_;    //external

// STScnSharedPtr (provides the retain/release model)

typedef void (*ScnSharedPtrDestroyOpqFunc)(void* opq);

struct STScnSharedPtr_* ScnSharedPtr_alloc(struct STScnContextItf_* ctx, ScnSharedPtrDestroyOpqFunc opqDestroyFunc, void* opq, const char* dbgHintStr);     //can be casted to void** to obtain the 'opq' value back
void    ScnSharedPtr_free(struct STScnSharedPtr_* obj);
void*   ScnSharedPtr_getOpq(struct STScnSharedPtr_* obj);
void    ScnSharedPtr_retain(struct STScnSharedPtr_* obj);
ScnSI32 ScnSharedPtr_release(struct STScnSharedPtr_* obj); //returns the retainCount

// STScnSharedPtr (provides retain/release model)

typedef struct STScnSharedPtr_ {
    void*           opq;        //opaque object, must be first member to allow toll-free casting
    ScnMutexRef   mutex;
    ScnSI32         retainCount;
    STScnMemoryItf  memItf;
    ScnSharedPtrDestroyOpqFunc opqDestroyFunc;
} STScnSharedPtr;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnSharedPtr_h */
