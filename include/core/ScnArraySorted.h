//
//  ScnArraySorted.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnArraySorted_h
#define ScnArraySorted_h

#include "ixtli-defs.h"
#include "core/ScnContext.h"
#include "core/ScnCompare.h"
#include <string.h> //memset

#ifdef __cplusplus
extern "C" {
#endif

#define ScnArraySortedStruct(NAME, TYPE)     \
struct { \
    TYPE        *arr; \
    ScnSI32     use; \
    ScnSI32     sz;  \
    ScnSI32     growth;  \
    NBCompareFunc cmpFunc; \
} NAME

#define ScnArraySorted_init(CTX_REF, ARRSTPTR, INITIAL_SZ, GROWTH, ARR_TYPE, CMP_FUNC) \
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
    (ARRSTPTR)->cmpFunc = CMP_FUNC; \
}

#define ScnArraySorted_destroy(CTX_REF, ARRSTPTR) \
{ \
    if((ARRSTPTR)->arr != NULL){ \
        ScnContext_mfree(CTX_REF, (ARRSTPTR)->arr); \
        (ARRSTPTR)->arr = NULL; \
    } \
    (ARRSTPTR)->sz = (ARRSTPTR)->use = 0; \
}

#define ScnArraySorted_empty(ARRSTPTR) \
{ \
    (ARRSTPTR)->use = 0; \
}

#define ScnArraySorted_indexForNew(DST, ARRSTPTR, ITM_PTR) \
{ \
    ScnSI32 r = -1; \
    SCN_ASSERT((ARRSTPTR)->cmpFunc != NULL) \
    if((ARRSTPTR)->cmpFunc != NULL){ \
        r = 0; \
        if((ARRSTPTR)->use > 0){ \
            ScnSI32 posStart    = 0; \
            ScnSI32 posEnd      = ((ARRSTPTR)->use - 1); \
            do { \
                if((*(ARRSTPTR)->cmpFunc)(ENCompareMode_LowerOrEqual, &(ARRSTPTR)->arr[posEnd], ITM_PTR, sizeof((ARRSTPTR)->arr[0]))){ \
                    r            = posEnd + 1; \
                    break; \
                } else if((*(ARRSTPTR)->cmpFunc)(ENCompareMode_GreaterOrEqual, &(ARRSTPTR)->arr[posStart], ITM_PTR, sizeof((ARRSTPTR)->arr[0]))){ \
                    r            = posStart; \
                    break; \
                } else { \
                    const ScnUI32 posMidd = (posStart + posEnd) / 2; \
                    if((*(ARRSTPTR)->cmpFunc)(ENCompareMode_LowerOrEqual, &(ARRSTPTR)->arr[posMidd], ITM_PTR, sizeof((ARRSTPTR)->arr[0]))){ \
                        posStart = posMidd + 1; \
                    } else { \
                        posEnd    = posMidd; \
                    } \
                } \
            } while(1); \
        } \
    } \
    DST = r; \
}

#define ScnArraySorted_indexOf(DST, ARRSTPTR, ITM_PTR) \
{ \
    ScnSI32 r = -1; \
    if((ARRSTPTR)->use > 0){ \
        ScnSI32 posEnd        = ((ARRSTPTR)->use - 1); \
        { \
            ScnSI32 posStart    = 0; \
            ScnSI32 posMidd; \
            const void* dataMidd = NULL; \
            SCN_ASSERT(sizeof((ARRSTPTR)->arr[0]) > 0) \
            while(posStart <= posEnd){ \
                posMidd        = posStart + ((posEnd - posStart)/2); \
                dataMidd    = &(ARRSTPTR)->arr[posMidd]; \
                if((*(ARRSTPTR)->cmpFunc)(ENCompareMode_Equal, dataMidd, ITM_PTR, sizeof((ARRSTPTR)->arr[0]))){ \
                    r = posMidd; \
                    break; \
                } else { \
                    if((*(ARRSTPTR)->cmpFunc)(ENCompareMode_Lower, ITM_PTR, dataMidd, sizeof((ARRSTPTR)->arr[0]))){ \
                        posEnd        = posMidd - 1; \
                    } else { \
                        posStart    = posMidd + 1; \
                    } \
                } \
            } \
        } \
    } \
    DST = r; \
}

#define ScnArraySorted_removeItemAtIndex(ARRSTPTR, INDEX) \
{ \
    ScnArraySorted_removeItemsAtIndex(ARRSTPTR, INDEX, 1); \
} \

#define ScnArraySorted_removeItemsAtIndex(ARRSTPTR, INDEX, COUNT) \
{ \
    ScnSI32 i = (INDEX), iCount, count = (COUNT); \
    SCN_ASSERT(i >= 0 && (i + count) <= (ARRSTPTR)->use) \
    (ARRSTPTR)->use -= count; \
    iCount = (ARRSTPTR)->use; \
    for(; i < iCount; i++){ \
        (ARRSTPTR)->arr[i] = (ARRSTPTR)->arr[i + count]; \
    } \
}

#define ScnArraySorted_addPtr(DST, CTX_REF, ARRSTPTR, ITM_PTR, ARR_TYPE) \
{ \
    if((ARRSTPTR)->use >= (ARRSTPTR)->sz){ \
        ScnSI32 szN = (ARRSTPTR)->use + ((ARRSTPTR)->growth > 0 ? (ARRSTPTR)->growth : 1); \
        ARR_TYPE* arrN = (ARR_TYPE*)ScnContext_mrealloc(CTX_REF, (ARRSTPTR)->arr, sizeof(ARR_TYPE) * szN, #ARRSTPTR); \
        if(arrN != NULL){ \
            (ARRSTPTR)->arr = arrN; \
            (ARRSTPTR)->sz  = szN; \
        } \
    } \
    DST = NULL; \
    if((ARRSTPTR)->use < (ARRSTPTR)->sz){ \
        ScnSI32 dstIndex = -1; \
        ScnArraySorted_indexForNew(dstIndex, ARRSTPTR, ITM_PTR); \
        SCN_ASSERT(dstIndex >= 0 && dstIndex <= (ARRSTPTR)->use) \
        if(dstIndex < (ARRSTPTR)->use){ \
            ScnSI32 i; for(i = ((ARRSTPTR)->use - 1); i >= dstIndex; i--){ \
                (ARRSTPTR)->arr[i + 1] = (ARRSTPTR)->arr[i]; \
            } \
        } \
        (ARRSTPTR)->arr[dstIndex] = *ITM_PTR; \
        (ARRSTPTR)->use++; \
        DST = &(ARRSTPTR)->arr[dstIndex]; \
    } \
}

#define ScnArraySorted_addValue(DST, CTX_REF, ARRSTPTR, ITM, ARR_TYPE) \
{ \
    if((ARRSTPTR)->use >= (ARRSTPTR)->sz){ \
        ScnSI32 szN = (ARRSTPTR)->use + ((ARRSTPTR)->growth > 0 ? (ARRSTPTR)->growth : 1); \
        ARR_TYPE* arrN = (ARR_TYPE*)ScnContext_mrealloc(CTX_REF, (ARRSTPTR)->arr, sizeof(ARR_TYPE) * szN, #ARRSTPTR); \
        if(arrN != NULL){ \
            (ARRSTPTR)->arr = arrN; \
            (ARRSTPTR)->sz  = szN; \
        } \
    } \
    DST = NULL; \
    if((ARRSTPTR)->use < (ARRSTPTR)->sz){ \
        ScnSI32 dstIndex = -1; \
        ScnArraySorted_indexForNew(dstIndex, ARRSTPTR, &(ITM)); \
        SCN_ASSERT(dstIndex >= 0 && dstIndex <= (ARRSTPTR)->use) \
        if(dstIndex < (ARRSTPTR)->use){ \
            ScnSI32 i; for(i = ((ARRSTPTR)->use - 1); i >= dstIndex; i--){ \
                (ARRSTPTR)->arr[i + 1] = (ARRSTPTR)->arr[i]; \
            } \
        } \
        (ARRSTPTR)->arr[dstIndex] = ITM; \
        (ARRSTPTR)->use++; \
        DST = &(ARRSTPTR)->arr[dstIndex]; \
    } \
}

#define ScnArraySorted_prepareForGrowth(CTX_REF, ARRSTPTR, AMM_NEW_ITEMS, ARR_TYPE) \
{ \
    const ScnSI32 qNewItems = AMM_NEW_ITEMS; \
    if (qNewItems > 0) { \
        const ScnSI32 nSz = (ARRSTPTR)->use + qNewItems; \
        if (nSz > (ARRSTPTR)->sz) { \
            ScnArraySorted_growBuffer(CTX_REF, ARRSTPTR, nSz - (ARRSTPTR)->sz, ARR_TYPE); \
        } \
    } \
}

#define ScnArraySorted_growBuffer(CTX_REF, ARRSTPTR, AMM_NEW_ITEMS, ARR_TYPE) \
{ \
    const ScnSI32 qNewItems = AMM_NEW_ITEMS; \
    if(qNewItems > 0){ \
        ScnSI32 szN = (ARRSTPTR)->sz + qNewItems; \
        ARR_TYPE* arrN = (ARR_TYPE*)ScnContext_mrealloc(CTX_REF, (ARRSTPTR)->arr, sizeof(ARR_TYPE) * szN, #ARRSTPTR); \
        if(arrN != NULL){ \
            (ARRSTPTR)->arr = arrN; \
            (ARRSTPTR)->sz  = szN; \
        } \
    } \
}

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnArraySorted_h */
