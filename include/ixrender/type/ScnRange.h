//
//  ScnRange.h
//  ixtli-render
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnRange_h
#define ScnRange_h

#include "ixrender/ixtli-defs.h"
#ifndef SNC_COMPILING_SHADER
#   include "ixrender/core/ScnCompare.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//STScnRange

/** @struct STScnRange
 *  @brief ScnFLOAT based range.
 *  @var STScnRange::start
 *  Range's start.
 *  @var STScnRange::size
 *  Range's size.
 */

#define STScnRange_Zero   { 0.f, 0.f }

typedef struct STScnRange {
    ScnFLOAT    start;
    ScnFLOAT    size;
} STScnRange;

#ifndef SNC_COMPILING_SHADER
ScnSI32 ScnCompare_STScnRange(const void* data1, const void* data2, const ScnUI32 dataSz);
#endif

//STScnRangeI

/** @struct STScnRangeI
 *  @brief ScnSI32 based range.
 *  @var STScnRangeI::start
 *  Range's start.
 *  @var STScnRangeI::size
 *  Range's size.
 */

#define STScnRangeI_Zero   { 0, 0 }

typedef struct STScnRangeI {
    ScnSI32    start;
    ScnSI32    size;
} STScnRangeI;

#ifndef SNC_COMPILING_SHADER
ScnSI32 ScnCompare_STScnRangeI(const void* data1, const void* data2, const ScnUI32 dataSz);
#endif

//STScnRangeU

/** @struct STScnRangeU
 *  @brief ScnUI32 based range.
 *  @var STScnRangeU::start
 *  Range's start.
 *  @var STScnRangeU::size
 *  Range's size.
 */

#define STScnRangeU_Zero   { 0u, 0u }

typedef struct STScnRangeU {
    ScnUI32    start;
    ScnUI32    size;
} STScnRangeU;

#ifndef SNC_COMPILING_SHADER
ScnSI32 ScnCompare_STScnRangeU(const void* data1, const void* data2, const ScnUI32 dataSz);
#endif

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRange_h */
