//
//  ScnContext.c
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/core/ScnContext.h"
#include "ixrender/core/ScnSharedPtr.h"
#include <string.h> //memset, memcpy

//ScnContextRef

typedef struct STScnContextOpq {
    ScnUI32 dummy;  //empty struct
} STScnContextOpq;

void ScnContext_destroyOpq_(void* opq);

ScnContextRef ScnContext_alloc(STScnContextItf* ctx){
    ScnContextRef r = ScnContextRef_Zero;
    if(ctx != NULL){
        STScnContextOpq* opq = (STScnContextOpq*)(ctx->mem.malloc)(sizeof(STScnContextOpq), "ScnContext_alloc::opq");
        STScnContextItf* itf = (STScnContextItf*)(ctx->mem.malloc)(sizeof(STScnContextItf), "ScnContext_alloc::itf");
        STScnSharedPtr* ptr  = ScnSharedPtr_alloc(ctx, ScnContext_destroyOpq_, opq, "ScnContext_alloc");
        if(opq != NULL && itf != NULL && ptr != NULL){
            memset(opq, 0, sizeof(*opq));
            memcpy(itf, ctx, sizeof(*itf));
            r.ptr = ptr; ptr = NULL; //consume
            r.itf = itf; itf = NULL; //consume
            opq = NULL; //consume
        }
        //release (if not consumed)
        if(opq != NULL){
            (ctx->mem.free)(opq);
            opq = NULL;
        }
        if(itf != NULL){
            (ctx->mem.free)(itf);
            itf = NULL;
        }
        if(ptr != NULL){
            ScnSharedPtr_free(ptr);
            ptr = NULL;
        }
    }
    return r;
}

void ScnContext_destroyOpq_(void* opq){
    //nothing
}

void ScnContext_retain(ScnContextRef ref){
    ScnSharedPtr_retain(ref.ptr);
}

void ScnContext_release(ScnContextRef* ref){
    if(ref != NULL && 0 == ScnSharedPtr_release(ref->ptr)){
        STScnContextOpq* opq = (STScnContextOpq*)ScnSharedPtr_getOpq(ref->ptr);
        ScnContextRef cpy = *ref;
        *ref = (ScnContextRef)ScnContextRef_Zero;
        //free
        if(opq != NULL){
            (cpy.itf->mem.free)(opq);
            opq = NULL;
        }
        if(cpy.itf != NULL){
            (cpy.itf->mem.free)(cpy.itf);
            cpy.itf = NULL;
        }
        if(cpy.ptr != NULL){
            ScnSharedPtr_free(cpy.ptr);
            cpy.ptr = NULL;
        }
    }
}

void ScnContext_releaseAndNull(ScnContextRef* ref){
    if(ref->ptr != NULL){
        ScnContext_release(ref);
        ref->ptr = NULL;
    }
}

void ScnContext_set(ScnContextRef* ref, ScnContextRef other){
    //retain first
    if(other.ptr != NULL){
        ScnContext_retain(other);
    }
    //release after
    if(ref->ptr != NULL){
        ScnContext_release(ref);
    }
    //set
    *ref = other;
}

void ScnContext_null(ScnContextRef* ref){
    *ref = (ScnContextRef)ScnContextRef_Zero;
}

//context (memory)

void* ScnContext_malloc(ScnContextRef ref, const ScnUI32 newSz, const char* dbgHintStr){
    return (ref.itf != NULL && ref.itf->mem.malloc != NULL ? (*ref.itf->mem.malloc)(newSz, dbgHintStr) : NULL);
}

void* ScnContext_mrealloc(ScnContextRef ref, void* ptr, const ScnUI32 newSz, const char* dbgHintStr){
    return (ref.itf != NULL && ref.itf->mem.realloc != NULL ? (*ref.itf->mem.realloc)(ptr, newSz, dbgHintStr) : NULL);
}

void ScnContext_mfree(ScnContextRef ref, void* ptr){
    if(ref.itf != NULL && ref.itf->mem.free != NULL){
        (*ref.itf->mem.free)(ptr);
    }
}

//context (mutex)

ScnMutexRef ScnContext_allocMutex(ScnContextRef ref){
    return (ref.itf != NULL && ref.itf->mutex.alloc != NULL ? (*ref.itf->mutex.alloc)(ref.itf) : (ScnMutexRef)ScnMutexRef_Zero);
}


//STScnContextItf (API)

STScnContextItf ScnContextItf_getDefault(void){
    STScnContextItf itf;
    memset(&itf, 0, sizeof(itf));
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
