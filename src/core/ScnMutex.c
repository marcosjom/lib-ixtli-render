//
//  ScnMutex.c
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ScnMutex.h"
#include "ScnContext.h"

//STScnMutexItf (API)

#if !defined(SCN_MUTEX_T) || !defined(SCN_MUTEX_INIT) || !defined(SCN_MUTEX_DESTROY) || !defined(SCN_MUTEX_LOCK) || !defined(SCN_MUTEX_UNLOCK)
#   ifdef _WIN32
//#     define WIN32_LEAN_AND_MEAN
#       include <windows.h>             //for CRITICAL_SECTION
#       define SCN_MUTEX_T              CRITICAL_SECTION
#       define SCN_MUTEX_INIT(PTR)      InitializeCriticalSection(PTR)
#       define SCN_MUTEX_DESTROY(PTR)   DeleteCriticalSection(PTR)
#       define SCN_MUTEX_LOCK(PTR)      EnterCriticalSection(PTR)
#       define SCN_MUTEX_UNLOCK(PTR)    LeaveCriticalSection(PTR)
#   else
#       include <pthread.h>             //for pthread_mutex_t
#       define SCN_MUTEX_T              pthread_mutex_t
#       define SCN_MUTEX_INIT(PTR)      pthread_mutex_init(PTR, NULL)
#       define SCN_MUTEX_DESTROY(PTR)   pthread_mutex_destroy(PTR)
#       define SCN_MUTEX_LOCK(PTR)      pthread_mutex_lock(PTR)
#       define SCN_MUTEX_UNLOCK(PTR)    pthread_mutex_unlock(PTR)
#   endif
#endif

// STScnMutexRef

void ScnMutex_lock(STScnMutexRef ref){
    if(ref.opq != NULL && ref.itf != NULL && ref.itf->lock != NULL){
        (*ref.itf->lock)(ref);
    }
}

void ScnMutex_unlock(STScnMutexRef ref){
    if(ref.opq != NULL && ref.itf != NULL && ref.itf->unlock != NULL){
        (*ref.itf->unlock)(ref);
    }
}

void ScnMutex_free(STScnMutexRef* ref){
    if(ref->opq != NULL && ref->itf != NULL && ref->itf->free != NULL){
        (*ref->itf->free)(ref);
    }
}

//STScnMutexOpq

typedef struct STScnMutexOpq_ {
    SCN_MUTEX_T             mutex;
    struct STScnContextItf_ ctx;
} STScnMutexOpq;

STScnMutexRef ScnMutexItf_default_alloc(struct STScnContextItf_* ctx){
    STScnMutexRef r = STScnMutexRef_Zero;
    STScnMutexOpq* obj = (STScnMutexOpq*)(*ctx->mem.malloc)(sizeof(STScnMutexOpq), "ScnMutexItf_default_alloc");
    if(obj != NULL){
        SCN_MUTEX_INIT(&obj->mutex);
        obj->ctx = *ctx;
        r.opq = obj;
        r.itf = &obj->ctx.mutex;
    }
    return r;
}

void ScnMutexItf_default_free(STScnMutexRef* pObj){
    if(pObj != NULL){
        STScnMutexOpq* obj = (STScnMutexOpq*)pObj->opq;
        if(obj != NULL){
            SCN_MUTEX_DESTROY(&obj->mutex);
            (*obj->ctx.mem.free)(obj);
        }
        pObj->opq = NULL;
        pObj->itf = NULL;
    }
}

void ScnMutexItf_default_lock(STScnMutexRef pObj){
    if(pObj.opq != NULL){
        STScnMutexOpq* obj = (STScnMutexOpq*)pObj.opq;
        SCN_MUTEX_LOCK(&obj->mutex);
    }
}

void ScnMutexItf_default_unlock(STScnMutexRef pObj){
    if(pObj.opq != NULL){
        STScnMutexOpq* obj = (STScnMutexOpq*)pObj.opq;
        SCN_MUTEX_UNLOCK(&obj->mutex);
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
