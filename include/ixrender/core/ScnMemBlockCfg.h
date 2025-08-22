//
//  ScnMemBlockCfg.h
//  ixtli-render
//
//  Created by Marcos Ortega on 21/8/25.
//

#ifndef ScnMemBlockCfg_h
#define ScnMemBlockCfg_h

#include "ixrender/ixtli-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnMemBlockCfg

/** @struct STScnMemBlockCfg
 *  @brief Memory block configuration.
 *  @var STScnMemBlockCfg::size
 *  Size of the block in bytes; the size never changes once the block is allocated.
 *  @var STScnMemBlockCfg::sizeAlign
 *  Alignment of the block's size. A block bigger that the requested size could be allocated to respect this alignment.
 *  @var STScnMemBlockCfg::idxsAlign
 *  Block's internal allocations aligment. A chunk bigger than the requjest size could be allocated to respect this alignment.
 *  @var STScnMemBlockCfg::isIdxZeroValid
 *  Determines if the idx==0 is a valid address for allocations; if not, idx=idxsAlign is the first address asignable.
 */

#define STScnMemBlockCfg_Zero { 0, 0, 0, ScnFALSE }

typedef struct STScnMemBlockCfg {
    ScnUI32 size;           //ammount of bytes allocable (including the idx-0)
    ScnUI32 sizeAlign;      //whole memory block size alignment
    ScnUI32 idxsAlign;      //individual pointers alignment
    ScnBOOL isIdxZeroValid; //idx=0 is an assignable address, if not, the first assignable address is 'idxsAlign * 1'.
} STScnMemBlockCfg;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnMemBlockCfg_h */
