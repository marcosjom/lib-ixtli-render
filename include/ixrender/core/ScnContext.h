//
//  ScnContext.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnContext_h
#define ScnContext_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnContextItf.h"
#include "ixrender/core/ScnMutex.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnContextRef

/** @struct ScnContextRef
 *  @brief ScnContext shared pointer. This is memory and mutexes manager. Every object in this library requires a ScnContext.
 *  @var ScnContextRef::ptr
 *  Shared pointer.
 *  @var ScnContextRef::itf
 *  Interface pointer.
 */

#define ScnContextRef_Zero    { NULL, NULL }

struct STScnSharedPtr;     //external

typedef struct ScnContextRef {
    struct STScnSharedPtr*  ptr;
    struct STScnContextItf* itf;
} ScnContextRef;

/**
 * @brief Allocates a new ScnContext using the specified interface.
 * @param ctx Context interface.
 * @return A new ScnContextRef on success, or ScnContextRef_Zero otherwise.
 */
ScnContextRef ScnContext_alloc(struct STScnContextItf* ctx);

/**
 * @brief Increases the reference's retain count.
 * @param ref Reference to object.
 */
void ScnContext_retain(ScnContextRef ref);

/**
 * @brief Decreases the reference's retain count, and frees the object if the retain count reached zero.
 * @param ref Reference to object.
 */
void ScnContext_release(ScnContextRef* ref);

/**
 * @brief Decreases the reference's retain count and nullyfies the reference. Frees the object if the retain count reached zero.
 * @note This is the equivalent of calling 'release' and 'null'.
 * @param ref Reference to object.
 */
void ScnContext_releaseAndNull(ScnContextRef* ref);

/**
 * @brief Sets the referenced object by retainning the new reference, and releasing the old reference afterwards.
 * @param ref Reference to object.
 * @param other Reference to new object.
 */
void ScnContext_set(ScnContextRef* ref, ScnContextRef other);

/**
 * @brief Compares two references.
 * @param ref Reference to object.
 * @param other Reference to other object.
 * @return ScnTRUE if both references points to the same object (including NULL), ScnFALSE otherwise.
 */
SC_INLN ScnBOOL ScnContext_isSame(ScnContextRef ref, ScnContextRef other) {
    return (ref.ptr == other.ptr);
}

/**
 * @brief Compares the references with NULL.
 * @param ref Reference to object.
 * @return ScnTRUE if references to NULL, ScnFALSE otherwise.
 */
SC_INLN ScnBOOL ScnContext_isNull(ScnContextRef ref) {
    return (ref.ptr == NULL);
}

/**
 * @brief Nullifiies/detaches the reference without releasing it.
 * @param ref Reference to object.
 */
void ScnContext_null(ScnContextRef* ref);

/**
 * @brief Allocates a block of memory.
 * @note The behaviour of this method should be the same as malloc().
 * @param ref Reference to context.
 * @param newSz Size of block to allocate.
 * @param dbgHintStr String used as hint for debugging memory leaks, optional.
 * @return A non-NULL pointer on success, or NULL otherwise.
 */
void* ScnContext_malloc(ScnContextRef ref, const ScnUI32 newSz, const char* dbgHintStr);

/**
 * @brief Reallocates a block of memory.
 * @note The behaviour of this method should be the same as realloc().
 * @param ref Reference to context.
 * @param ptr Original pointer to reallocate; can be NULL.
 * @param newSz Size of block to allocate.
 * @param dbgHintStr String used as hint for debugging memory leaks, optional.
 * @return A non-NULL pointer on success, or NULL otherwise.
 */
void* ScnContext_mrealloc(ScnContextRef ref, void* ptr, const ScnUI32 newSz, const char* dbgHintStr);

/**
 * @brief Frees a block of memory.
 * @note The behaviour of this method should be the same as free().
 * @param ref Reference to context.
 * @param ptr Original pointer to free.
 */
void ScnContext_mfree(ScnContextRef ref, void* ptr);

/**
 * @brief Allocates a new ScnMutex using the provided ScnContext.
 * @param ref Reference to context.
 * @return A new ScnMutexRef on success, or ScnMutexRef_Zero otherwise.
 */
ScnMutexRef ScnContext_allocMutex(ScnContextRef ref);


#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnContext_h */
