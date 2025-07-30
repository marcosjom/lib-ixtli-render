//
//  ScnMemBlock.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnMemBlock_h
#define ScnMemBlock_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnAbsPtr, abstract pointer

#define STScnAbsPtr_Zero { NULL, 0 }

typedef struct STScnAbsPtr_ {
    void*       ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    ScnUI32     idx;    //abstract address
    //Note: possible 4-bytes padding here.
} STScnAbsPtr;

//STScnMemBlockCfg

#define STScnMemBlockCfg_Zero { 0, 0, 0, ScnFALSE }

typedef struct STScnMemBlockCfg_ {
    ScnUI32 size;           //ammount of bytes allocable (including the idx-0)
    ScnUI32 sizeAlign;      //whole memory block size alignment
    ScnUI32 idxsAlign;      //individual pointers alignment
    ScnBOOL idxZeroIsValid; //idx=0 is an assignable address
} STScnMemBlockCfg;

//ScnMemBlockRef

SCN_REF_STRUCT_METHODS_DEC(ScnMemBlock)

ScnBOOL     ScnMemBlock_prepare(ScnMemBlockRef ref, const STScnMemBlockCfg* cfg, STScnAbsPtr* dstPtrAfterEnd);
ScnBOOL     ScnMemBlock_hasPtrs(ScnMemBlockRef ref); //allocations made?
ScnUI32     ScnMemBlock_getAddressableSize(ScnMemBlockRef ref); //includes the address zero
STScnAbsPtr ScnMemBlock_getStarAddress(ScnMemBlockRef ref); //includes the address zero
//allocations
STScnAbsPtr ScnMemBlock_malloc(ScnMemBlockRef ref, const ScnUI32 usableSz);
ScnBOOL     ScnMemBlock_mfree(ScnMemBlockRef ref, const STScnAbsPtr ptr);
ScnUI32     ScnMemBlock_mAvailSz(ScnMemBlockRef ref);
//
void        ScnMemBlock_prepareForNewMallocsActions(ScnMemBlockRef ref, const ScnUI32 ammActions);   //increases the index's sz
void        ScnMemBlock_clear(ScnMemBlockRef ref); //clears the index, all pointers are invalid after this call
//dbg
ScnBOOL     ScnMemBlock_validateIndex(ScnMemBlockRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnMemBlock_h */
