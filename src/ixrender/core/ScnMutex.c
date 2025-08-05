//
//  ScnMutex.c
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/core/ScnMutex.h"
#include "ixrender/core/ScnContext.h"

//STScnMutexItf (API)

#if !defined(Scn_MUTEX_T) || !defined(Scn_MUTEX_INIT) || !defined(Scn_MUTEX_DESTROY) || !defined(Scn_MUTEX_LOCK) || !defined(Scn_MUTEX_UNLOCK)
#   ifdef _WIN32
//#     define WIN32_LEAN_AND_MEAN
#       include <windows.h>             //for CRITICAL_SECTION
#       define Scn_MUTEX_T              CRITICAL_SECTION
#       define Scn_MUTEX_INIT(PTR)      InitializeCriticalSection(PTR)
#       define Scn_MUTEX_DESTROY(PTR)   DeleteCriticalSection(PTR)
#       define Scn_MUTEX_LOCK(PTR)      EnterCriticalSection(PTR)
#       define Scn_MUTEX_UNLOCK(PTR)    LeaveCriticalSection(PTR)
#   else
#       include <pthread.h>             //for pthread_mutex_t
#       define Scn_MUTEX_T              pthread_mutex_t
#       define Scn_MUTEX_INIT(PTR)      pthread_mutex_init(PTR, NULL)
#       define Scn_MUTEX_DESTROY(PTR)   pthread_mutex_destroy(PTR)
#       define Scn_MUTEX_LOCK(PTR)      pthread_mutex_lock(PTR)
#       define Scn_MUTEX_UNLOCK(PTR)    pthread_mutex_unlock(PTR)
#   endif
#endif

// ScnMutexRef

void ScnMutex_lock(ScnMutexRef ref){
    if(ref.opq != NULL && ref.itf != NULL && ref.itf->lock != NULL){
        (*ref.itf->lock)(ref);
    }
}

void ScnMutex_unlock(ScnMutexRef ref){
    if(ref.opq != NULL && ref.itf != NULL && ref.itf->unlock != NULL){
        (*ref.itf->unlock)(ref);
    }
}

void ScnMutex_free(ScnMutexRef* ref){
    if(ref->opq != NULL && ref->itf != NULL && ref->itf->free != NULL){
        (*ref->itf->free)(ref);
    }
}

//STScnMutexOpq

typedef struct STScnMutexOpq {
    Scn_MUTEX_T         mutex;
    STScnContextItf     ctx;
} STScnMutexOpq;

ScnMutexRef ScnMutexItf_default_alloc(struct STScnContextItf* ctx){
    ScnMutexRef r = ScnMutexRef_Zero;
    STScnMutexOpq* obj = (STScnMutexOpq*)(*ctx->mem.malloc)(sizeof(STScnMutexOpq), "ScnMutexItf_default_alloc");
    if(obj != NULL){
        Scn_MUTEX_INIT(&obj->mutex);
        obj->ctx = *ctx;
        r.opq = obj;
        r.itf = &obj->ctx.mutex;
    }
    return r;
}

void ScnMutexItf_default_free(ScnMutexRef* pObj){
    if(pObj != NULL){
        STScnMutexOpq* obj = (STScnMutexOpq*)pObj->opq;
        if(obj != NULL){
            Scn_MUTEX_DESTROY(&obj->mutex);
            (*obj->ctx.mem.free)(obj);
        }
        pObj->opq = NULL;
        pObj->itf = NULL;
    }
}

void ScnMutexItf_default_lock(ScnMutexRef pObj){
    if(pObj.opq != NULL){
        STScnMutexOpq* obj = (STScnMutexOpq*)pObj.opq;
        Scn_MUTEX_LOCK(&obj->mutex);
    }
}

void ScnMutexItf_default_unlock(ScnMutexRef pObj){
    if(pObj.opq != NULL){
        STScnMutexOpq* obj = (STScnMutexOpq*)pObj.opq;
        Scn_MUTEX_UNLOCK(&obj->mutex);
    }
}

//Links NULL methods to a DEFAULT implementation,
//this reduces the need to check for functions NULL pointers.
void ScnMutexItf_fillMissingMembers(STScnMutexItf* itf){
    if(itf == NULL) return;
    SCN_ITF_SET_MISSING_METHOD_TO_DEFAULT(itf, ScnMutexItf, alloc);
    SCN_ITF_SET_MISSING_METHOD_TO_DEFAULT(itf, ScnMutexItf, free);
    SCN_ITF_SET_MISSING_METHOD_TO_DEFAULT(itf, ScnMutexItf, lock);
    SCN_ITF_SET_MISSING_METHOD_TO_DEFAULT(itf, ScnMutexItf, unlock);
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
