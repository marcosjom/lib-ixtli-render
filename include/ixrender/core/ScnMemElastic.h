//
//  ScnMemElastic.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnMemElastic_h
#define ScnMemElastic_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/core/ScnMemBlock.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnMemBlockCfg

#define STScnMemElasticCfg_Zero { 0, 0, 0, 0, 0, ScnFALSE }

typedef struct STScnMemElasticCfg {
    ScnUI32 sizePerBlock;   //ammount of bytes allocable per block (including the idx-0)
    ScnUI32 sizeInitial;    //memory to allocate initially
    ScnUI32 sizeMax;        //max allowed size in bytes (0 is infinite)
    ScnUI32 sizeAlign;      //whole memory block size alignment
    ScnUI32 idxsAlign;      //individual pointers alignment
    ScnBOOL idxZeroIsValid; //idx=0 is an assignable address
} STScnMemElasticCfg;

//ScnMemElasticRef

SCN_REF_STRUCT_METHODS_DEC(ScnMemElastic)

//

ScnBOOL     ScnMemElastic_prepare(ScnMemElasticRef ref, const STScnMemElasticCfg* cfg, ScnUI32* optDstBlocksTotalSz);
ScnBOOL     ScnMemElastic_hasPtrs(ScnMemElasticRef ref); //allocations made?
ScnUI32     ScnMemElastic_getAddressableSize(ScnMemElasticRef ref); //includes the address zero
STScnAbsPtr ScnMemElastic_getNextContinuousAddress(ScnMemElasticRef ref, const ScnUI32 iAddress, ScnUI32* dstContinuousSz);
//allocations
STScnAbsPtr ScnMemElastic_malloc(ScnMemElasticRef ref, const ScnUI32 usableSz, ScnUI32* optDstBlocksTotalSz);
ScnBOOL     ScnMemElastic_mfree(ScnMemElasticRef ref, const STScnAbsPtr ptr);
//
void        ScnMemElastic_clear(ScnMemElasticRef ref); //clears the index, all pointers are invalid after this call
//dbg
ScnBOOL     ScnMemElastic_validateIndex(ScnMemElasticRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnMemElastic_h */
