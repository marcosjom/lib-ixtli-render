//
//  ScnSharedPtr.c
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/core/ScnSharedPtr.h"
#include "ixrender/core/ScnMemory.h"
#include "ixrender/core/ScnMutex.h"
#include "ixrender/core/ScnContext.h"

// STScnSharedPtr (provides retain/release model)

struct STScnSharedPtr_* ScnSharedPtr_alloc(STScnContextItf* itf, ScnSharedPtrDestroyOpqFunc opqDestroyFunc, void* opq, const char* dbgHintStr){
    struct STScnSharedPtr_* obj = NULL;
    obj = (struct STScnSharedPtr_*)(*itf->mem.malloc)(sizeof(struct STScnSharedPtr_), dbgHintStr);
    if(obj != NULL){
        obj->mutex = (itf->mutex.alloc)(itf);
        obj->retainCount = 1; //retained by creator
        //
        obj->opq = opq;
        obj->memItf = itf->mem;
        obj->opqDestroyFunc = opqDestroyFunc;
    }
    return obj;
}

void ScnSharedPtr_free(struct STScnSharedPtr_* obj){
    ScnMutex_free(&obj->mutex);
    (*obj->memItf.free)(obj);
}

void* ScnSharedPtr_getOpq(struct STScnSharedPtr_* obj){
    //can also be obtained by casting 'obj' as '*(void**)obj'
    return (obj != NULL ? obj->opq : NULL);
}

void ScnSharedPtr_retain(struct STScnSharedPtr_* obj){
    ScnMutex_lock(obj->mutex);
    {
        SCN_ASSERT(obj->retainCount > 0) //if fails, the pointer was re-activated during cleanup (change your code to avoid this)
        ++obj->retainCount;
    }
    ScnMutex_unlock(obj->mutex);
    /*
#   ifdef SCN_DEBUG
    if(obj->dbgItf.retainedBy != NULL){
        const STScnDbgThreadState* st = ScnDbgThreadState_get();
        (*obj->dbgItf.retainedBy)(obj, (st != NULL && st->stack.use > 0 ? st->stack.arr[st->stack.use - 1].className.str : NULL));
    }
#   endif
    */
}

ScnSI32 ScnSharedPtr_release(struct STScnSharedPtr_* obj){
    ScnSI32 r = 0;
    ScnMutex_lock(obj->mutex);
    {
        SCN_ASSERT(obj->retainCount > 0)
        r = --obj->retainCount;
    }
    ScnMutex_unlock(obj->mutex);
    //destroy and free opq
    if(r == 0){
        if(obj->opqDestroyFunc != NULL){
            (*obj->opqDestroyFunc)(obj->opq);
        }
        if(obj->opq != NULL){
            (*obj->memItf.free)(obj->opq);
        }
        obj->opqDestroyFunc = NULL;
        obj->opq = NULL;
    }
    /*
#   ifdef SCN_DEBUG
    if(obj->dbgItf.releasedBy != NULL){
        const STScnDbgThreadState* st = ScnDbgThreadState_get();
        (*obj->dbgItf.releasedBy)(obj, (st != NULL && st->stack.use > 0 ? st->stack.arr[st->stack.use - 1].className.str : NULL));
    }
#   endif
     */
    return r;
}
