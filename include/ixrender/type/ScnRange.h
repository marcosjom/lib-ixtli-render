//
//  ScnRange.h
//  ixtli-render
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnRange_h
#define ScnRange_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnCompare.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnRange

#define STScnRange_Zero   { 0.f, 0.f }

typedef struct STScnRange {
    ScnFLOAT    start;
    ScnFLOAT    size;
} STScnRange;

ScnBOOL ScnCompare_STScnRange(const ENScnCompareMode mode, const void* data1, const void* data2, const ScnUI32 dataSz);

//STScnRangeI

#define STScnRangeI_Zero   { 0, 0 }

typedef struct STScnRangeI {
    ScnSI32    start;
    ScnSI32    size;
} STScnRangeI;

ScnBOOL ScnCompare_STScnRangeI(const ENScnCompareMode mode, const void* data1, const void* data2, const ScnUI32 dataSz);

//STScnRangeU

#define STScnRangeU_Zero   { 0u, 0u }

typedef struct STScnRangeU {
    ScnUI32    start;
    ScnUI32    size;
} STScnRangeU;

ScnBOOL ScnCompare_STScnRangeU(const ENScnCompareMode mode, const void* data1, const void* data2, const ScnUI32 dataSz);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRange_h */
