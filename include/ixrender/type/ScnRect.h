//
//  ScnRect.h
//  ixtli-render
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnRect_h
#define ScnRect_h

#include "ixrender/ixtli-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

// STScnRect

/** @struct STScnRect
 *  @brief 2d ScnFLOAT rectangle.
 *  @var STScnRect::x
 *  X position
 *  @var STScnRect::y
 *  Y position
 *  @var STScnRect::width
 *  X width
 *  @var STScnRect::height
 *  Y height
 */

#define STScnRect_Zero { 0.f, 0.f, 0.f, 0.f }

typedef struct STScnRect {
    ScnFLOAT    x;
    ScnFLOAT    y;
    ScnFLOAT    width;
    ScnFLOAT    height;
} STScnRect;

// STScnRectI

/** @struct STScnRectI
 *  @brief 2d ScnSI32 rectangle.
 *  @var STScnRectI::x
 *  X position
 *  @var STScnRectI::y
 *  Y position
 *  @var STScnRectI::width
 *  X width
 *  @var STScnRectI::height
 *  Y height
 */

#define STScnRectI_Zero { 0, 0, 0, 0 }

typedef struct STScnRectI {
    ScnSI32     x;
    ScnSI32     y;
    ScnSI32     width;
    ScnSI32     height;
} STScnRectI;

// STScnRectU

/** @struct STScnRectU
 *  @brief 2d ScnUI32 rectangle.
 *  @var STScnRectU::x
 *  X position
 *  @var STScnRectU::y
 *  Y position
 *  @var STScnRectU::width
 *  X width
 *  @var STScnRectU::height
 *  Y height
 */

#define STScnRectU_Zero { 0u, 0u, 0u, 0u }

typedef struct STScnRectU {
    ScnUI32     x;
    ScnUI32     y;
    ScnUI32     width;
    ScnUI32     height;
} STScnRectU;

// STScnRectI16

/** @struct STScnRectI16
 *  @brief 2d ScnSI16 rectangle.
 *  @var STScnRectI16::x
 *  X position
 *  @var STScnRectI16::y
 *  Y position
 *  @var STScnRectI16::width
 *  X width
 *  @var STScnRectI16::height
 *  Y height
 */

#define STScnRectI16_Zero { 0, 0, 0, 0 }

typedef struct STScnRectI16 {
    ScnSI16     x;
    ScnSI16     y;
    ScnSI16     width;
    ScnSI16     height;
} STScnRectI16;

// STScnRectU16

/** @struct STScnRectU16
 *  @brief 2d ScnUI16 rectangle.
 *  @var STScnRectU16::x
 *  X position
 *  @var STScnRectU16::y
 *  Y position
 *  @var STScnRectU16::width
 *  X width
 *  @var STScnRectU16::height
 *  Y height
 */

#define STScnRectU16_Zero { 0u, 0u, 0u, 0u }

typedef struct STScnRectU16 {
    ScnUI16     x;
    ScnUI16     y;
    ScnUI16     width;
    ScnUI16     height;
} STScnRectU16;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRect_h */
