//
//  ScnArrayEmbed.h
//  ixtli-render
//
//  Created by Marcos Ortega on 2/8/25.
//

#ifndef ScnArrayEmbed_h
#define ScnArrayEmbed_h

#include "ixrender/core/ScnArray.h"

/**
 * @brief An optimized array that shares the same space between one embedded slot and a array at heap. Great for cases where zero-or-one item is stored most of the time.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Macro that defines an struct with the necesary variables for an unordered array.
 */

#define ScnArrayEmbedStruct(NAME, TYPE)     \
    struct { \
        ScnBOOL                 areMany; \
        union { \
            struct { \
                ScnBOOL         isSet; \
                ScnUI32         growth; \
                TYPE            one; \
            } embedded; \
            ScnArrayStruct(many, TYPE); \
        }; \
    } NAME

/**
 * @brief Macro that initializes an unordered array.
 */

#define ScnArrayEmbed_init(CTX_REF, ARRSTPTR, INITIAL_SZ, GROWTH, ARR_TYPE) \
    if(INITIAL_SZ <= 1){ \
        (ARRSTPTR)->areMany = ScnFALSE; \
        (ARRSTPTR)->embedded.isSet   = ScnFALSE; \
        (ARRSTPTR)->embedded.growth  = GROWTH; \
    } else { \
        (ARRSTPTR)->areMany = ScnTRUE; \
        ScnArray_init(CTX_REF, &(ARRSTPTR)->many, INITIAL_SZ, GROWTH, ARR_TYPE); \
    }

/**
 * @brief Macro that destroys an unordered array.
 */

#define ScnArrayEmbed_destroy(CTX_REF, ARRSTPTR) \
    if((ARRSTPTR)->areMany){ \
        ScnArray_destroy(CTX_REF, &(ARRSTPTR)->many); \
        (ARRSTPTR)->areMany = ScnFALSE; \
        (ARRSTPTR)->embedded.isSet = ScnFALSE; \
    }

/**
 * @brief Macro to get the current use of the unordered array.
 */

#define ScnArrayEmbed_getUse(ARRSTPTR) \
    ((ARRSTPTR)->areMany ? (ARRSTPTR)->many.use : (ARRSTPTR)->embedded.isSet ? 1 : 0)

/**
 * @brief Macro to get the pointer to the elements of the unordered array.
 */
#define ScnArrayEmbed_getItmsPtr(ARRSTPTR) \
    ((ARRSTPTR)->areMany ? (ARRSTPTR)->many.arr : (ARRSTPTR)->embedded.isSet ? &(ARRSTPTR)->embedded.one : NULL)

/**
 * @brief Macro that empties an unordered array.
 */

#define ScnArrayEmbed_empty(ARRSTPTR) \
    if(!(ARRSTPTR)->areMany){ \
        (ARRSTPTR)->embedded.isSet = ScnFALSE; \
    } else { \
        ScnArray_empty(&(ARRSTPTR)->many); \
    }

/**
 * @brief Macro that adds an element to an unordered array.
 */

#define ScnArrayEmbed_addPtr(PTRDST, CTX_REF, ARRSTPTR, ITM_PTR, ARR_TYPE) \
    PTRDST = NULL; \
    if(!(ARRSTPTR)->areMany){ \
        if(!(ARRSTPTR)->embedded.isSet){ \
            (ARRSTPTR)->embedded.one = *(ITM_PTR); \
            (ARRSTPTR)->embedded.isSet = ScnTRUE; \
            PTRDST = &(ARRSTPTR)->embedded.one; \
        } else { \
            ARR_TYPE cpy = (ARRSTPTR)->embedded.one; \
            ScnUI32 growth = ((ARRSTPTR)->embedded.growth <= 0 ? 8 : (ARRSTPTR)->embedded.growth); \
            (ARRSTPTR)->areMany = ScnTRUE; \
            ScnArray_init(CTX_REF, &(ARRSTPTR)->many, growth, growth, ARR_TYPE); \
            if(NULL == ScnArray_addPtr(CTX_REF, &(ARRSTPTR)->many, &cpy, ARR_TYPE)){ \
                ScnArray_destroy(CTX_REF, &(ARRSTPTR)->many); \
                (ARRSTPTR)->areMany         = ScnFALSE; \
                (ARRSTPTR)->embedded.isSet  = ScnTRUE; \
                (ARRSTPTR)->embedded.growth = growth; \
                (ARRSTPTR)->embedded.one    = cpy; \
            } \
        } \
    } \
    if((ARRSTPTR)->areMany){ \
        PTRDST = ScnArray_addPtr(CTX_REF, &(ARRSTPTR)->many, ITM_PTR, ARR_TYPE); \
    }

/**
 * @brief Macro that walks all the elements of an unordered array.
 */

#define ScnArrayEmbed_foreach(ARRSTPTR, ARR_TYPE, VAR_NAME, ...) \
    if(!(ARRSTPTR)->areMany){ \
        if((ARRSTPTR)->embedded.isSet){ \
            ARR_TYPE* VAR_NAME = &(ARRSTPTR)->embedded.one; \
            __VA_ARGS__ \
        } \
    } else { \
        ScnArray_foreach(&(ARRSTPTR)->many, ARR_TYPE, VAR_NAME, \
            __VA_ARGS__ \
        ); \
    }

#endif /* ScnArrayEmbed_h */
