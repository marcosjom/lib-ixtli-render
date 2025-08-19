//
//  ScnArraySorted.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnArraySorted_h
#define ScnArraySorted_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnContext.h"
#include "ixrender/core/ScnCompare.h"

/**
 * @brief An sorted array.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Macro that defines an struct with the necesary variables for an sorted array.
 */

#define ScnArraySortedStruct(NAME, TYPE)     \
struct { \
    TYPE        *arr; \
    ScnSI32     use; \
    ScnSI32     sz;  \
    ScnSI32     growth;  \
    ScnCompareFunc cmpFunc; \
} NAME

/**
 * @brief Macro that initializes an sorted array.
 */

#define ScnArraySorted_init(CTX_REF, ARRSTPTR, INITIAL_SZ, GROWTH, ARR_TYPE, CMP_FUNC) \
{ \
    ScnMemory_setZeroSt(*(ARRSTPTR)); \
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

/**
 * @brief Macro that destroys an sorted array.
 */

#define ScnArraySorted_destroy(CTX_REF, ARRSTPTR) \
{ \
    if((ARRSTPTR)->arr != NULL){ \
        ScnContext_mfree(CTX_REF, (ARRSTPTR)->arr); \
        (ARRSTPTR)->arr = NULL; \
    } \
    (ARRSTPTR)->sz = (ARRSTPTR)->use = 0; \
}

/**
 * @brief Macro that empties an sorted array.
 */
#define ScnArraySorted_empty(ARRSTPTR) \
{ \
    (ARRSTPTR)->use = 0; \
}

/**
 * @brief Macro that gets the insert position for the provided external element for the sorted array.
 */
#define ScnArraySorted_indexForNew(ARRSTPTR, ITM_PTR) \
        ScnArraySorted_indexForNew_((ARRSTPTR)->arr, (ARRSTPTR)->use, sizeof((ARRSTPTR)->arr[0]), ITM_PTR, sizeof(*ITM_PTR), (ARRSTPTR)->cmpFunc)

ScnSI32 ScnArraySorted_indexForNew_(const void* arr, const ScnSI32 use, const ScnSI32 arrItmSz, const void* itmPtr, const ScnSI32 itmSz, ScnCompareFunc cmpFunc);

/**
 * @brief Macro that gets the position in the sorted sorted of the element that matches the provided value.
 */

#define ScnArraySorted_indexOf(ARRSTPTR, ITM_PTR) \
        ScnArraySorted_indexOf_((ARRSTPTR)->arr, (ARRSTPTR)->use, sizeof((ARRSTPTR)->arr[0]), ITM_PTR, sizeof(*ITM_PTR), (ARRSTPTR)->cmpFunc)

ScnSI32 ScnArraySorted_indexOf_(const void* pArr, const ScnSI32 use, const ScnSI32 arrItmSz, const void* itmPtr, const ScnSI32 itmSz, ScnCompareFunc cmpFunc);

/**
 * @brief Macro that removes an item from the sorted array.
 */

#define ScnArraySorted_removeItemAtIndex(ARRSTPTR, INDEX) \
{ \
    ScnArraySorted_removeItemsAtIndex(ARRSTPTR, INDEX, 1); \
} \

/**
 * @brief Macro that removes multiple items from the sorted array.
 */

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

/**
 * @brief Macro that adds an item into the sorted array in its sorted position.
 */

#define ScnArraySorted_addPtr(CTX_REF, ARRSTPTR, ITM_PTR, ARR_TYPE) \
        (ARR_TYPE*)ScnArraySorted_addPtr_(CTX_REF, (void**)&(ARRSTPTR)->arr, &(ARRSTPTR)->use, &(ARRSTPTR)->sz, (ARRSTPTR)->growth, sizeof((ARRSTPTR)->arr[0]), ITM_PTR, sizeof(*(ITM_PTR)), (ARRSTPTR)->cmpFunc, SCN_DBG_STR("ScnArraySorted_addPtr::" #ARRSTPTR))

void*   ScnArraySorted_addPtr_(ScnContextRef ctx, void** pArr, ScnSI32* use, ScnSI32* sz, const ScnSI32 growth, const ScnSI32 arrItmSz, const void* itmPtr, const ScnSI32 itmSz, ScnCompareFunc cmpFunc, const char* dbgHint);

/**
 * @brief Macro that ensures enought space in the sorted array for this future ammount of insertions.
 */

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

/**
 * @brief Macro that grows the buffer of the sorted array.
 */

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

/**
 * @brief Macro that walks all the elements of an sorted array.
 */

#define ScnArraySorted_foreach(ARRSTPTR, ARR_TYPE, VAR_NAME, ...) \
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

#endif /* ScnArraySorted_h */
