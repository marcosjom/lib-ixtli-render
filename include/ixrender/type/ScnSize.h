//
//  ScnSize.h
//  ixtli-render
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnSize_h
#define ScnSize_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnCompare.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STScnSize2D_Zero { 0.f, 0.f }

typedef struct STScnSize2D_ {
    ScnFLOAT    width;
    ScnFLOAT    height;
} STScnSize2D;

//

#define STScnSize2DI_Zero { 0, 0 }

typedef struct STScnSize2DI_ {
    ScnSI32    width;
    ScnSI32    height;
} STScnSize2DI;

//

#define STScnSize2DU_Zero { 0u, 0u }

typedef struct STScnSize2DU_ {
    ScnUI32    width;
    ScnUI32    height;
} STScnSize2DU;

//

#define STScnSize2DI16_Zero { 0, 0 }

typedef struct STScnSize2DI16_ {
    ScnSI16    width;
    ScnSI16    height;
} STScnSize2DI16;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnSize_h */
