//
//  ScnAbsPtr.h
//  ixtli-render
//
//  Created by Marcos Ortega on 21/8/25.
//

#ifndef ScnAbsPtr_h
#define ScnAbsPtr_h

#include "ixrender/ixtli-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnAbsPtr, abstract pointer

/** @struct STScnAbsPtr
 *  @brief Describes a reserved memory chunk.
 *  @var STScnAbsPtr::ptr
 *  Pointer to memory. This pointer is an absolute address and should be valid until freed.
 *  @var STScnAbsPtr::idx
 *  Relative index; index zero is the first address.
 *  @var STScnAbsPtr::itfParam
 *  Opaque object to which this chunk belongs.
 */

#define STScnAbsPtr_Zero { NULL, 0, NULL }

typedef struct STScnAbsPtr {
    void*       ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    ScnUI32     idx;    //abstract address
    void*       itfParam; //object to wich the memory block belongs (the ScnMemBlock itself if no custom itf was provided)
} STScnAbsPtr;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnAbsPtr_h */
