//
//  ScnAABBox3d.h
//  ixtli-render
//
//  Created by Marcos Ortega on 2/8/25.
//

#ifndef ScnAABBox3d_h
#define ScnAABBox3d_h

#include "ixrender/type/ScnRange.h"

//Axis Aligned Bounding Box

#ifdef __cplusplus
extern "C" {
#endif

// STScnAABBox2d

/** @struct STScnAABBox2d
 *  @brief Axis aligned 2d box.
 *  @var STScnAABBox2d::x
 *  X coordinates min and max value.
 *  @var STScnAABBox2d::y
 *  Y coordinates min and max value.
 */

#define STScnAABBox2d_Zero      { STScnRngLimits_Zero, STScnRngLimits_Zero }
#define STScnAABBox2d_Identity  { STScnRngLimits_Identity, STScnRngLimits_Identity }

typedef struct STScnAABBox2d {
    STScnRngLimits  x;
    STScnRngLimits  y;
} STScnAABBox2d;

// STScnAABBox3d

/** @struct STScnAABBox3d
 *  @brief Axis aligned 3d box.
 *  @var STScnAABBox2d::x
 *  X coordinates min and max value.
 *  @var STScnAABBox2d::y
 *  Y coordinates min and max value.
 *  @var STScnAABBox2d::z
 *  Z coordinates min and max value.
 */

#define STScnAABBox3d_Zero      { STScnRngLimits_Zero, STScnRngLimits_Zero, STScnRngLimits_Zero }
#define STScnAABBox3d_Identity  { STScnRngLimits_Identity, STScnRngLimits_Identity, STScnRngLimits_Identity }

typedef struct STScnAABBox3d {
    STScnRngLimits  x;
    STScnRngLimits  y;
    STScnRngLimits  z;
} STScnAABBox3d;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnAABBox3d_h */
