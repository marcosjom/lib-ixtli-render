//
//  ScnArraySorted.c
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#include "ixrender/core/ScnArraySorted.h"

ScnSI32 ScnArraySorted_indexForNew_(const void* pArr, const ScnSI32 use, const ScnSI32 arrItmSz, const void* itmPtr, const ScnSI32 itmSz, ScnCompareFunc cmpFunc){
    SCN_ASSERT(cmpFunc != NULL)
    SCN_ASSERT(arrItmSz == itmSz)
    //
    if(cmpFunc == NULL) return 0;
    if(arrItmSz != itmSz) return 0;
    if(use <= 0) return 0;
    //
    const ScnBYTE* arr  = (const ScnBYTE*)pArr;
    ScnSI32 posStart    = 0;
    ScnSI32 posEnd      = use - 1;
    do {
        if((*cmpFunc)(ENScnCompareMode_LowerOrEqual, &arr[posEnd * itmSz], itmPtr, itmSz)){
            return posEnd + 1;
        } else if((*cmpFunc)(ENScnCompareMode_GreaterOrEqual, &arr[posStart * itmSz], itmPtr, itmSz)){
            return posStart;
        } else {
            const ScnUI32 posMidd = (posStart + posEnd) / 2;
            if((*cmpFunc)(ENScnCompareMode_LowerOrEqual, &arr[posMidd * itmSz], itmPtr, itmSz)){
                posStart = posMidd + 1;
            } else {
                posEnd    = posMidd;
            }
        }
    } while(1);
    //should never reach here
    SCN_ASSERT(0);
    return 0;
}

ScnSI32 ScnArraySorted_indexOf_(const void* pArr, const ScnSI32 use, const ScnSI32 arrItmSz, const void* itmPtr, const ScnSI32 itmSz, ScnCompareFunc cmpFunc){
    SCN_ASSERT(cmpFunc != NULL)
    SCN_ASSERT(arrItmSz == itmSz)
    //
    if(cmpFunc == NULL) return -1;
    if(arrItmSz != itmSz) return -1;
    if(use <= 0) return -1;
    //
    const ScnBYTE* arr  = (const ScnBYTE*)pArr;
    const void* dataMidd = NULL;
    ScnSI32 posEnd      = use - 1;
    ScnSI32 posStart    = 0;
    ScnSI32 posMidd;
    while(posStart <= posEnd){
        posMidd         = posStart + ((posEnd - posStart) / 2);
        dataMidd        = &arr[posMidd * itmSz];
        if((*cmpFunc)(ENScnCompareMode_Equal, dataMidd, itmPtr, itmSz)){
            return posMidd;
        } else {
            if((*cmpFunc)(ENScnCompareMode_Lower, itmPtr, dataMidd, itmSz)){
                posEnd  = posMidd - 1;
            } else {
                posStart = posMidd + 1;
            }
        }
    }
    //
    return -1;
}

void* ScnArraySorted_addPtr_(ScnContextRef ctx, void** arr, ScnSI32* use, ScnSI32* sz, const ScnSI32 growth, const ScnSI32 arrItmSz, const void* itmPtr, const ScnSI32 itmSz, ScnCompareFunc cmpFunc, const char* dbgHint){
    SCN_ASSERT(!(arr == NULL || use == NULL || sz == NULL || cmpFunc == NULL))
    SCN_ASSERT(arrItmSz == itmSz)
    //
    if(arr == NULL || use == NULL || sz == NULL  || cmpFunc == NULL) return NULL;
    if(arrItmSz != itmSz) return NULL;
    //
    if(*use >= *sz){
        const ScnSI32 szN = *use + (growth > 0 ? growth : 1);
        void* arrN = ScnContext_mrealloc(ctx, *arr, arrItmSz * szN, dbgHint);
        if(arrN != NULL){
            *arr = arrN;
            *sz  = szN;
        }
    }
    if(*use < *sz){
        ScnBYTE* bArr = (ScnBYTE*)*arr;
        const ScnSI32 dstIndex = ScnArraySorted_indexForNew_(*arr, *use, arrItmSz, itmPtr, itmSz, cmpFunc);
        SCN_ASSERT(dstIndex >= 0 && dstIndex <= *use)
        if(dstIndex < *use){
            //create gap
            memmove(&bArr[(dstIndex + 1) * arrItmSz], &bArr[dstIndex * arrItmSz], (*use - dstIndex) * arrItmSz);
        }
        memcpy(&bArr[dstIndex * arrItmSz], itmPtr, itmSz);
        *use = *use + 1;
        return &bArr[dstIndex * arrItmSz];
    }
    //
    return NULL;
}
