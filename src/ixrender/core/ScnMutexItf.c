//
//  ScnMutexItf.c
//  ixtli-render
//
//  Created by Marcos Ortega on 21/8/25.
//

#include "ixrender/core/ScnMutexItf.h"
#include "ixrender/core/ScnContextItf.h"
#include "ixrender/core/ScnMutex.h"

//STScnMutexItf (default implementation)

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

//STScnMutexOpq

typedef struct STScnMutexOpq {
    SCN_MUTEX_T         mutex;
    STScnContextItf     ctx;
} STScnMutexOpq;

ScnMutexRef ScnMutexItf_default_alloc(struct STScnContextItf* ctx){
    ScnMutexRef r = ScnMutexRef_Zero;
    STScnMutexOpq* obj = (STScnMutexOpq*)(*ctx->mem.malloc)(sizeof(STScnMutexOpq), SCN_DBG_STR("ScnMutexItf_default_alloc"));
    if(obj != NULL){
        SCN_MUTEX_INIT(&obj->mutex);
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
            SCN_MUTEX_DESTROY(&obj->mutex);
            (*obj->ctx.mem.free)(obj);
        }
        pObj->opq = NULL;
        pObj->itf = NULL;
    }
}

void ScnMutexItf_default_lock(ScnMutexRef pObj){
    if(pObj.opq != NULL){
        STScnMutexOpq* obj = (STScnMutexOpq*)pObj.opq;
        SCN_MUTEX_LOCK(&obj->mutex);
    }
}

void ScnMutexItf_default_unlock(ScnMutexRef pObj){
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
