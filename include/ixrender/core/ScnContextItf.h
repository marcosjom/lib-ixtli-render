//
//  ScnContextItf.h
//  ixtli-render
//
//  Created by Marcos Ortega on 21/8/25.
//

#ifndef ScnContextItf_h
#define ScnContextItf_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnMemoryItf.h"
#include "ixrender/core/ScnMutexItf.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnContextItf

/** @struct STScnContextItf
 *  @brief Context interface.
 *  @var STScnContextItf::mem
 *  Memory allocation interface.
 *  @var STScnContextItf::mutex
 *  Mutexes interface.
 */

typedef struct STScnContextItf {
    STScnMemoryItf  mem;
    STScnMutexItf   mutex;
} STScnContextItf;

STScnContextItf ScnContextItf_getDefault(void);

/**
 * @brief Links the passed interface NULL methods to a DEFAULT implementation. This reduces the need to check for functions NULL pointers.
 * @param itf Interface to be populated.
 */
void ScnContextItf_fillMissingMembers(STScnContextItf* itf);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnContextItf_h */
