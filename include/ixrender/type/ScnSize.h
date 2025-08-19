//
//  ScnSize.h
//  ixtli-render
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnSize_h
#define ScnSize_h

#include "ixrender/ixtli-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

// STScnSize2D

/** @struct STScnSize2D
 *  @brief 2d ScnFLOAT size.
 *  @var STScnSize2D::width
 *  X width
 *  @var STScnSize2D::height
 *  Y height
 */

#define STScnSize2D_Zero { 0.f, 0.f }

typedef struct STScnSize2D {
    ScnFLOAT    width;
    ScnFLOAT    height;
} STScnSize2D;

// STScnSize2DI

/** @struct STScnSize2DI
 *  @brief 2d ScnSI32 size.
 *  @var STScnSize2DI::width
 *  X width
 *  @var STScnSize2DI::height
 *  Y height
 */

#define STScnSize2DI_Zero { 0, 0 }

typedef struct STScnSize2DI {
    ScnSI32    width;
    ScnSI32    height;
} STScnSize2DI;

// STScnSize2DU

/** @struct STScnSize2DU
 *  @brief 2d ScnUI32 size.
 *  @var STScnSize2DU::width
 *  X width
 *  @var STScnSize2DU::height
 *  Y height
 */

#define STScnSize2DU_Zero { 0u, 0u }

typedef struct STScnSize2DU {
    ScnUI32    width;
    ScnUI32    height;
} STScnSize2DU;

// STScnSize2DI16

/** @struct STScnSize2DI16
 *  @brief 2d ScnSI16 size.
 *  @var STScnSize2DI16::width
 *  X width
 *  @var STScnSize2DI16::height
 *  Y height
 */

#define STScnSize2DI16_Zero { 0, 0 }

typedef struct STScnSize2DI16 {
    ScnSI16    width;
    ScnSI16    height;
} STScnSize2DI16;

// STScnSize2DU16

/** @struct STScnSize2DU16
 *  @brief 2d ScnUI16 size.
 *  @var STScnSize2DU16::width
 *  X width
 *  @var STScnSize2DU16::height
 *  Y height
 */

#define STScnSize2DU16_Zero { 0u, 0u }

typedef struct STScnSize2DU16 {
    ScnUI16    width;
    ScnUI16    height;
} STScnSize2DU16;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnSize_h */
