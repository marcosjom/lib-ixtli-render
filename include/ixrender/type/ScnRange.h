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

#define STScnRange_Zero   { 0.f, 0.f }

typedef struct STScnRange {
    ScnFLOAT    start;
    ScnFLOAT    size;
} STScnRange;

#ifndef SNC_COMPILING_SHADER
ScnSI32 ScnCompare_STScnRange(const void* data1, const void* data2, const ScnUI32 dataSz);
#endif

//STScnRangeI

#define STScnRangeI_Zero   { 0, 0 }

typedef struct STScnRangeI {
    ScnSI32    start;
    ScnSI32    size;
} STScnRangeI;

#ifndef SNC_COMPILING_SHADER
ScnSI32 ScnCompare_STScnRangeI(const void* data1, const void* data2, const ScnUI32 dataSz);
#endif

//STScnRangeU

#define STScnRangeU_Zero   { 0u, 0u }

typedef struct STScnRangeU {
    ScnUI32    start;
    ScnUI32    size;
} STScnRangeU;

#ifndef SNC_COMPILING_SHADER
ScnSI32 ScnCompare_STScnRangeU(const void* data1, const void* data2, const ScnUI32 dataSz);
#endif

// STScnAABBox3d

#define STScnRngLimits_Zero     { 0.f, 0.f }
#define STScnRngLimits_Identity { 0.f, 1.f }

typedef struct STScnRngLimits {
    ScnFLOAT    min;
    ScnFLOAT    max;
} STScnRngLimits;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRange_h */
