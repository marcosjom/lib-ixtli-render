//
//  ScnMutex.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnMutex_h
#define ScnMutex_h

#include "ixrender/ixtli-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

struct STScnMutexItf; //external

// ScnMutexRef

/** @struct ScnMutexRef
 *  @brief ScnMutex pointer.
 */

#define ScnMutexRef_Zero { NULL, NULL }

typedef struct ScnMutexRef {
    void*                   opq;
    struct STScnMutexItf*   itf;
} ScnMutexRef;

/**
 * @brief Compares the pointer with NULL.
 * @param ref Reference to object.
 * @return ScnTRUE if references to NULL, ScnFALSE otherwise.
 */
#define ScnMutex_isNull(REF)                ((REF).opq != NULL)

/**
 * @brief Nullifiies the pointer without freeing it.
 * @param ref Reference to object.
 */
#define ScnMutex_null(REF_PTR)              (REF_PTR)->opq = NULL

/**
 * @brief Frees the pointer and nullyfies it afterwards.
 * @note This is the equivalent of calling 'free' and 'null'.
 * @param ref Reference to object.
 */
#define ScnMutex_freeAndNullify(REF_PTR)    if((REF_PTR)->opq != NULL){ ScnMutex_free(REF_PTR); (REF_PTR)->opq = NULL; }

/**
 * @brief Frees the object.
 * @param ref Reference to object.
 */
void ScnMutex_free(ScnMutexRef* ref);

/**
 * @brief Activates the mutex.
 * @param ref Reference to object.
 */
void ScnMutex_lock(ScnMutexRef ref);

/**
 * @brief Deactivates the mutex.
 * @param ref Reference to object.
 */
void ScnMutex_unlock(ScnMutexRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnMutex_h */
