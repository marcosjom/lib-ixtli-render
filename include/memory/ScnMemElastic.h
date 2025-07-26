//
//  ScnMemElastic.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnMemElastic_h
#define ScnMemElastic_h

#include "ixtli-defs.h"
#include "core/ScnContext.h"
#include "core/ScnSharedPtr.h"
#include "memory/ScnMemBlock.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnMemBlockCfg

#define STScnMemElasticCfg_Zero { 0, 0, 0, 0, 0, SCN_FALSE }

typedef struct STScnMemElasticCfg_ {
    ScnUI32 sizePerBlock;   //ammount of bytes allocable per block (including the idx-0)
    ScnUI32 sizeInitial;    //memory to allocate initially
    ScnUI32 sizeMax;        //max allowed size in bytes (0 is infinite)
    ScnUI32 sizeAlign;      //whole memory block size alignment
    ScnUI32 idxsAlign;      //individual pointers alignment
    ScnBOOL idxZeroIsValid; //idx=0 is an assignable address
} STScnMemElasticCfg;

//STScnMemElasticRef

#define STScnMemElasticRef_Zero    { NULL }

typedef struct STScnMemElasticRef_ {
    STScnSharedPtr* ptr;
} STScnMemElasticRef;

SCN_STRUCT_REF_METHODS_DEC(ScnMemElastic)

//

ScnBOOL     ScnMemElastic_prepare(STScnMemElasticRef ref, const STScnMemElasticCfg* cfg, ScnUI32* optDstBlocksTotalSz);
//allocations
STScnAbsPtr ScnMemElastic_malloc(STScnMemElasticRef ref, const ScnUI32 usableSz, ScnUI32* optDstBlocksTotalSz);
ScnBOOL     ScnMemElastic_mfree(STScnMemElasticRef ref, const STScnAbsPtr ptr);
//
void        ScnMemElastic_clear(STScnMemElasticRef ref); //clears the index, all pointers are invalid after this call
//dbg
ScnBOOL     ScnMemElastic_validateIndex(STScnMemElasticRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnMemElastic_h */
