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

#define STScnSize2D_Zero { 0.f, 0.f }

typedef struct STScnSize2D {
    ScnFLOAT    width;
    ScnFLOAT    height;
} STScnSize2D;

// STScnSize2DI

#define STScnSize2DI_Zero { 0, 0 }

typedef struct STScnSize2DI {
    ScnSI32    width;
    ScnSI32    height;
} STScnSize2DI;

// STScnSize2DU

#define STScnSize2DU_Zero { 0u, 0u }

typedef struct STScnSize2DU {
    ScnUI32    width;
    ScnUI32    height;
} STScnSize2DU;

// STScnSize2DI16

#define STScnSize2DI16_Zero { 0, 0 }

typedef struct STScnSize2DI16 {
    ScnSI16    width;
    ScnSI16    height;
} STScnSize2DI16;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnSize_h */
