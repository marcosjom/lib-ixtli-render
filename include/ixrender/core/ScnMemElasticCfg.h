//
//  ScnMemElasticCfg.h
//  ixtli-render
//
//  Created by Marcos Ortega on 21/8/25.
//

#ifndef ScnMemElasticCfg_h
#define ScnMemElasticCfg_h

#include "ixrender/ixtli-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnMemBlockCfg

/** @struct STScnMemElasticCfg
 *  @brief Elastic memory blocks-chain configuration.
 *  @var STScnMemElasticCfg::sizePerBlock
 *  Size per memory block in the elastic chain.
 *  @var STScnMemElasticCfg::sizeInitial
 *  Minimal initial size in bytes to be allocated in the elastic memory block.
 *  @var STScnMemElasticCfg::sizeMax
 *  Maximun size in bytes to be allocated in the elastic memory block, zero means unlimited.
 *  @var STScnMemElasticCfg::sizeAlign
 *  Alignment of each block's size.
 *  @var STScnMemElasticCfg::idxsAlign
 *  Block's internal allocations aligment. A chunk bigger than the requjest size could be allocated to respect this alignment.
 *  @var STScnMemElasticCfg::isIdxZeroValid
 *  Determines if the idx==0 is a valid address for allocations; if not, idx=idxsAlign is the first address asignable.
 */

#define STScnMemElasticCfg_Zero { 0, 0, 0, 0, 0, ScnFALSE }

typedef struct STScnMemElasticCfg {
    ScnUI32 sizePerBlock;   //ammount of bytes allocable per block (including the idx-0)
    ScnUI32 sizeInitial;    //memory to allocate initially
    ScnUI32 sizeMax;        //max allowed size in bytes (0 is infinite)
    ScnUI32 sizeAlign;      //whole memory block size alignment
    ScnUI32 idxsAlign;      //individual pointers alignment
    ScnBOOL isIdxZeroValid; //idx=0 is an assignable address
} STScnMemElasticCfg;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnMemElasticCfg_h */
