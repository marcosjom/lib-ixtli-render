//
//  ScnArray.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnArray_h
#define ScnArray_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnContext.h"
#include <string.h> //memset

#ifdef __cplusplus
extern "C" {
#endif

#define ScnArrayStruct(NAME, TYPE)     \
    struct { \
        TYPE        *arr; \
        ScnSI32     use; \
        ScnSI32     sz;  \
        ScnSI32     growth;  \
    } NAME

#define ScnArray_init(CTX_REF, ARRSTPTR, INITIAL_SZ, GROWTH, ARR_TYPE) \
    { \
        memset(ARRSTPTR, 0, sizeof(*ARRSTPTR)); \
        if(INITIAL_SZ > 0){ \
            ARR_TYPE* arrN = (ARR_TYPE*)ScnContext_malloc(CTX_REF, sizeof(ARR_TYPE) * (INITIAL_SZ), #ARRSTPTR); \
            if(arrN != NULL){ \
                (ARRSTPTR)->arr = arrN; \
                (ARRSTPTR)->sz  = INITIAL_SZ; \
            } \
        } \
        (ARRSTPTR)->growth = GROWTH; \
    }

#define ScnArray_destroy(CTX_REF, ARRSTPTR) \
    { \
        if((ARRSTPTR)->arr != NULL){ \
            ScnContext_mfree(CTX_REF, (ARRSTPTR)->arr); \
            (ARRSTPTR)->arr = NULL; \
        } \
        (ARRSTPTR)->sz = (ARRSTPTR)->use = 0; \
    }

#define ScnArray_empty(ARRSTPTR)  (ARRSTPTR)->use = 0

#define ScnArray_addValue(CTX_REF, ARRSTPTR, ITM, ARR_TYPE) \
    { \
        if((ARRSTPTR)->use >= (ARRSTPTR)->sz){ \
            ScnSI32 szN = (ARRSTPTR)->use + ((ARRSTPTR)->growth > 0 ? (ARRSTPTR)->growth : 1); \
            ARR_TYPE* arrN = (ARR_TYPE*)ScnContext_mrealloc(CTX_REF, (ARRSTPTR)->arr, sizeof(ARR_TYPE) * szN, #ARRSTPTR); \
            if(arrN != NULL){ \
                (ARRSTPTR)->arr = arrN; \
                (ARRSTPTR)->sz  = szN; \
            } \
        } \
        if((ARRSTPTR)->use < (ARRSTPTR)->sz){ \
            (ARRSTPTR)->arr[(ARRSTPTR)->use++] = ITM; \
        } \
    }

#define ScnArray_addPtr(CTX_REF, ARRSTPTR, ITM_PTR, ARR_TYPE) \
        (ARR_TYPE*) ScnArray_addPtr_(CTX_REF, (void**)&(ARRSTPTR)->arr, &(ARRSTPTR)->use, &(ARRSTPTR)->sz, (ARRSTPTR)->growth, sizeof((ARRSTPTR)->arr[0]), ITM_PTR, sizeof(*(ITM_PTR)), "ScnArraySorted_addPtr::" #ARRSTPTR)

void*   ScnArray_addPtr_(ScnContextRef ctx, void** pArr, ScnSI32* use, ScnSI32* sz, const ScnSI32 growth, const ScnSI32 arrItmSz, const void* itmPtr, const ScnSI32 itmSz, const char* dbgHint);


#define ScnArray_foreach(ARRSTPTR, ARR_TYPE, VAR_NAME, ...) \
    if((ARRSTPTR)->arr != NULL){ \
        ARR_TYPE* VAR_NAME = (ARRSTPTR)->arr; \
        const ARR_TYPE* VAR_NAME ## AfterEnd = VAR_NAME + (ARRSTPTR)->use; \
        while(VAR_NAME < VAR_NAME ## AfterEnd){ \
            __VA_ARGS__ \
            ++VAR_NAME; \
        } \
    }

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnArray_h */
