//
//  ScnMutex.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnMutex_h
#define ScnMutex_h

#include "ixrender/ixtli-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ScnMutexRef;
struct STScnMutexItf;
//
struct STScnContextItf; //external

// ScnMutexRef

#define ScnMutexRef_Zero { NULL, NULL }

typedef struct ScnMutexRef {
    void*                   opq;
    struct STScnMutexItf*   itf;
} ScnMutexRef;

#define ScnMutex_isNull(REF)                ((REF).opq != NULL)
#define ScnMutex_null(REF_PTR)              (REF_PTR)->opq = NULL
#define ScnMutex_freeAndNullify(REF_PTR)    if((REF_PTR)->opq != NULL){ ScnMutex_free(REF_PTR); (REF_PTR)->opq = NULL; }
void    ScnMutex_free(ScnMutexRef* ref);
void    ScnMutex_lock(ScnMutexRef ref);
void    ScnMutex_unlock(ScnMutexRef ref);

//STScnMutexItf

typedef struct STScnMutexItf {
    ScnMutexRef (*alloc)(struct STScnContextItf* ctx);
    void        (*free)(ScnMutexRef* obj);
    void        (*lock)(ScnMutexRef obj);
    void        (*unlock)(ScnMutexRef obj);
} STScnMutexItf;

//Links NULL methods to a DEFAULT implementation,
//this reduces the need to check for functions NULL pointers.
void ScnMutexItf_fillMissingMembers(STScnMutexItf* itf);


#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnMutex_h */
