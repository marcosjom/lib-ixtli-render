//
//  ScnArray.c
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/core/ScnArray.h"

void* ScnArray_addPtr_(STScnContextRef ctx, void** arr, ScnSI32* use, ScnSI32* sz, const ScnSI32 growth, const ScnSI32 arrItmSz, const void* itmPtr, const ScnSI32 itmSz, const char* dbgHint){
    SCN_ASSERT(!(arr == NULL || use == NULL || sz == NULL))
    SCN_ASSERT(arrItmSz == itmSz)
    //
    if(arr == NULL || use == NULL || sz == NULL) return NULL;
    if(arrItmSz != itmSz) return NULL;
    if(*use >= *sz){
        ScnSI32 szN = *use + (growth > 0 ? growth : 1);
        void* arrN = ScnContext_mrealloc(ctx, *arr, arrItmSz * szN, dbgHint);
        if(arrN != NULL){
            *arr = arrN;
            *sz  = szN;
        }
    }
    if(*use < *sz){
        void* ptr = &((ScnBYTE*)*arr)[arrItmSz * (*use)];
        memcpy(ptr, itmPtr, itmSz);
        *use = *use + 1;
        return ptr;
    }
    return NULL;
}
