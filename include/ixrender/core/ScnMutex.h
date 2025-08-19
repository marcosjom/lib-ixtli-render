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

struct ScnMutexRef;
struct STScnMutexItf;
//
struct STScnContextItf; //external

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

//STScnMutexItf

/** @struct STScnMutexItf
 *  @brief Interface for mutex allocation and use. This allows to link this library to your own management's core.
 *  @var STScnMutexItf::alloc
 *  Function to allocate a new mutex.
 *  @var STScnMutexItf::free
 *  Function to free a previously allocated mutex.
 *  @var STScnMutexItf::lock
 *  Function to activate the mutex.
 *  @var STScnMutexItf::unlock
 *  Function to deactivate the mutex.
 */

typedef struct STScnMutexItf {
    ScnMutexRef (*alloc)(struct STScnContextItf* ctx);
    void        (*free)(ScnMutexRef* obj);
    void        (*lock)(ScnMutexRef obj);
    void        (*unlock)(ScnMutexRef obj);
} STScnMutexItf;

/**
 * @brief Links the provided interface's NULL methods to a DEFAULT implementation, this reduces the need to check for functions NULL pointers.
 * @note Passing a zero-intialized interface will provide the default implementation in return.
 * @param itf The interface to be populated
 */
void ScnMutexItf_fillMissingMembers(STScnMutexItf* itf);


#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnMutex_h */
