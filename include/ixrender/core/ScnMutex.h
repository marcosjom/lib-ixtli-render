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

struct STScnMutexRef_;
struct STScnMutexItf_;
//
struct STScnContextItf_; //external

// STScnMutexRef

#define STScnMutexRef_Zero { NULL, NULL }

typedef struct STScnMutexRef_ {
    void*                   opq;
    struct STScnMutexItf_*  itf;
} STScnMutexRef;

#define ScnMutex_isNull(REF)                ((REF).opq != NULL)
#define ScnMutex_null(REF_PTR)              (REF_PTR)->opq = NULL
#define ScnMutex_freeAndNullify(REF_PTR)    if((REF_PTR)->opq != NULL){ ScnMutex_free(REF_PTR); (REF_PTR)->opq = NULL; }
void    ScnMutex_free(STScnMutexRef* ref);
void    ScnMutex_lock(STScnMutexRef ref);
void    ScnMutex_unlock(STScnMutexRef ref);

//STScnMutexItf

typedef struct STScnMutexItf_ {
    STScnMutexRef (*alloc)(struct STScnContextItf_* ctx);
    void        (*free)(STScnMutexRef* obj);
    void        (*lock)(STScnMutexRef obj);
    void        (*unlock)(STScnMutexRef obj);
} STScnMutexItf;

//Links NULL methods to a DEFAULT implementation,
//this reduces the need to check for functions NULL pointers.
void ScnMutexItf_fillMissingMembers(STScnMutexItf* itf);


#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnMutex_h */
