//
//  ScnRngLimits.h
//  ixtli-render
//
//  Created by Marcos Ortega on 21/8/25.
//

#ifndef ScnLimits_h
#define ScnLimits_h

#include "ixrender/ixtli-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

// STScnRngLimits

/** @struct STScnRngLimits
 *  @brief ScnFLOAT based limits.
 *  @note Min/max sides are relative to the context, it could mean a mathematical order or left-is-min, right-is-max.
 *  @var STScnRangeU::min
 *  Limit's min side value.
 *  @var STScnRangeU::max
 *  Limit's max side value.
 */

#define STScnRngLimits_Zero     { 0.f, 0.f }
#define STScnRngLimits_Identity { 0.f, 1.f }

typedef struct STScnRngLimits {
    ScnFLOAT    min;
    ScnFLOAT    max;
} STScnRngLimits;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnLimits_h */
