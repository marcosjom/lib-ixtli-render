//
//  ScnMemBlockPtr.h
//  ixtli-render
//
//  Created by Marcos Ortega on 21/8/25.
//

#ifndef ScnMemBlockPtr_h
#define ScnMemBlockPtr_h

#include "ixrender/ixtli-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnMemBlockPtr

/** @struct STScnMemBlockPtr
 *  @brief Describes a block of allocated memory.
 *  @var STScnMemBlockPtr::ptr
 *  Pointer to memory.
 *  @var STScnMemBlockPtr::sz
 *  Size of memory block.
 */

#define STScnMemBlockPtr_Zero { NULL, 0 }

typedef struct STScnMemBlockPtr {
    void*       ptr;  //pointer returned by 'ScnMemBlock_malloc'
    ScnUI32     sz;   //size at 'ScnMemBlock_malloc' call
} STScnMemBlockPtr;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnMemBlockPtr_h */
