//
//  ScnMutexItf.h
//  ixtli-render
//
//  Created by Marcos Ortega on 21/8/25.
//

#ifndef ScnMutexItf_h
#define ScnMutexItf_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnMutex.h"

#ifdef __cplusplus
extern "C" {
#endif

struct STScnContextItf; //external

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

#endif /* ScnMutexItf_h */
