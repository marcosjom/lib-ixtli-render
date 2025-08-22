//
//  ScnMutex.c
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/core/ScnMutex.h"
#include "ixrender/core/ScnMutexItf.h"

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

