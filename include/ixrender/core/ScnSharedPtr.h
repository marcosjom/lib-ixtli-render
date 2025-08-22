//
//  ScnSharedPtr.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnSharedPtr_h
#define ScnSharedPtr_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnMemoryItf.h"
#include "ixrender/core/ScnMutex.h"

#ifdef __cplusplus
extern "C" {
#endif

struct STScnContextItf;    //external

// STScnSharedPtr (provides the retain/release model)

typedef void (*ScnSharedPtrDestroyOpqFunc)(void* opq);

// STScnSharedPtr (provides retain/release model)

/** @struct STScnSharedPtr
 *  @brief Shared pointer.
 *  @var STScnSharedPtr::opq
 *  Opaque internal pointer.
 *  @var STScnSharedPtr::mutex
 *  Mutex for the retain-count.
 *  @var STScnSharedPtr::retainCount
 *  Retain count.
 *  @var STScnSharedPtr::memItf
 *  Memory allocator interface.
 *  @var STScnSharedPtr::opqDestroyFunc
 *  Function to be called to destroy and free the opaque pointer.
 */
typedef struct STScnSharedPtr {
    void*           opq;        //opaque object, must be first member to allow toll-free casting
    ScnMutexRef     mutex;
    ScnSI32         retainCount;
    STScnMemoryItf  memItf;
    ScnSharedPtrDestroyOpqFunc opqDestroyFunc;
} STScnSharedPtr;

/**
 * @brief Allocates a shared pointer.
 * @param ctx ScnContext interface to be used.
 * @param opqDestroyFunc Function for destruction and freeing of the allocated pointer.
 * @param opq Parameter to be passed ot the destruction function.
 * @param optDbgHintStr String used as debuggin hint, optinal.
 * @return Allocated pointer on success, NULL otherwise.
 */
struct STScnSharedPtr* ScnSharedPtr_alloc(struct STScnContextItf* ctx, ScnSharedPtrDestroyOpqFunc opqDestroyFunc, void* opq, const char* optDbgHintStr);

/**
 * @brief Frees a previously allocated shared pointer.
 * @param obj Reference to object.
 */
void ScnSharedPtr_free(struct STScnSharedPtr* obj);

/**
 * @brief Retrieves the opaque internal pointer.
 * @param obj Reference to object.
 * @return The opaque internal pointer, NULL otherwise.
 */
void* ScnSharedPtr_getOpq(struct STScnSharedPtr* obj);

/**
 * @brief Increases the shared pointer retain-count.
 * @param obj Reference to object.
 */
void ScnSharedPtr_retain(struct STScnSharedPtr* obj);

/**
 * @brief Decreases the shared pointer retain-count.
 * @param obj Reference to object.
 * @return The retain-count after the operation.
 */
ScnSI32 ScnSharedPtr_release(struct STScnSharedPtr* obj);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnSharedPtr_h */
