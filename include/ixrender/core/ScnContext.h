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

struct ScnContextRef_;
struct STScnContextItf_;
//
struct STScnSharedPtr_;     //external

//ScnContextRef

#define ScnContextRef_Zero    { NULL, NULL }

typedef struct ScnContextRef_ {
    struct STScnSharedPtr_*  ptr;
    struct STScnContextItf_* itf;
} ScnContextRef;

ScnContextRef ScnContext_alloc(struct STScnContextItf_* ctx);
void            ScnContext_retain(ScnContextRef ref);
void            ScnContext_release(ScnContextRef* ref);
void            ScnContext_releaseAndNull(ScnContextRef* ref);
void            ScnContext_set(ScnContextRef* ref, ScnContextRef other);
SC_INLN ScnBOOL ScnContext_isSame(ScnContextRef ref, ScnContextRef other) { return (ref.ptr == other.ptr); }
SC_INLN ScnBOOL ScnContext_isNull(ScnContextRef ref) { return (ref.ptr == NULL); }
void            ScnContext_null(ScnContextRef* ref);
//context (memory)
void*           ScnContext_malloc(ScnContextRef ref, const ScnUI32 newSz, const char* dbgHintStr);
void*           ScnContext_mrealloc(ScnContextRef ref, void* ptr, const ScnUI32 newSz, const char* dbgHintStr);
void            ScnContext_mfree(ScnContextRef ref, void* ptr);
//context (mutex)
ScnMutexRef   ScnContext_allocMutex(ScnContextRef ref);

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
