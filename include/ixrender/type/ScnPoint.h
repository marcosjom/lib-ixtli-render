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

typedef struct STScnPoint2D {
    ScnFLOAT    x;
    ScnFLOAT    y;
} STScnPoint2D;

typedef struct STScnPoint2DI {
    ScnSI32     x;
    ScnSI32     y;
} STScnPoint2DI;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnPoint_h */
