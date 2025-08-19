//
//  ScnPoint.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnPoint_h
#define ScnPoint_h

#include "ixrender/ixtli-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnPoint2D

/** @struct STScnPoint2D
 *  @brief 2d ScnFLOAT point.
 *  @var STScnPoint2D::x
 *  X position
 *  @var STScnPoint2D::y
 *  Y position
 */
typedef struct STScnPoint2D {
    ScnFLOAT    x;
    ScnFLOAT    y;
} STScnPoint2D;

//STScnPoint2DI

/** @struct STScnPoint2DI
 *  @brief 2d ScnSI32 point.
 *  @var STScnPoint2DI::x
 *  X position
 *  @var STScnPoint2DI::y
 *  Y position
 */
typedef struct STScnPoint2DI {
    ScnSI32     x;
    ScnSI32     y;
} STScnPoint2DI;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnPoint_h */
