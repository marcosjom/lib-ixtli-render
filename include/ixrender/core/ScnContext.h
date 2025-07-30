//
//  ScnContext.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnContext_h
#define ScnContext_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnMemory.h"
#include "ixrender/core/ScnMutex.h"

#ifdef __cplusplus
extern "C" {
#endif

struct STScnContextRef_;
struct STScnContextItf_;
//
struct STScnSharedPtr_;     //external

//STScnContextRef

#define STScnContextRef_Zero    { NULL, NULL }

typedef struct STScnContextRef_ {
    struct STScnSharedPtr_*  ptr;
    struct STScnContextItf_* itf;
} STScnContextRef;

STScnContextRef ScnContext_alloc(struct STScnContextItf_* ctx);
void            ScnContext_retain(STScnContextRef ref);
void            ScnContext_release(STScnContextRef* ref);
void            ScnContext_set(STScnContextRef* ref, STScnContextRef other);
SC_INLN ScnBOOL ScnContext_isSame(STScnContextRef ref, STScnContextRef other) { return (ref.ptr == other.ptr); }
SC_INLN ScnBOOL ScnContext_isNull(STScnContextRef ref) { return (ref.ptr == NULL); }
void            ScnContext_null(STScnContextRef* ref);
//context (memory)
void*           ScnContext_malloc(STScnContextRef ref, const ScnUI32 newSz, const char* dbgHintStr);
void*           ScnContext_mrealloc(STScnContextRef ref, void* ptr, const ScnUI32 newSz, const char* dbgHintStr);
void            ScnContext_mfree(STScnContextRef ref, void* ptr);
//context (mutex)
STScnMutexRef   ScnContext_allocMutex(STScnContextRef ref);

//STScnContextItf

typedef struct STScnContextItf_ {
    STScnMemoryItf  mem;
    STScnMutexItf   mutex;
} STScnContextItf;

STScnContextItf ScnContextItf_getDefault(void);

//Links NULL methods to a DEFAULT implementation,
//this reduces the need to check for functions NULL pointers.
void ScnContextItf_fillMissingMembers(STScnContextItf* itf);


#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnContext_h */
