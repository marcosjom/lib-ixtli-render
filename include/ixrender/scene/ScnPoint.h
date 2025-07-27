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

typedef struct STScnPoint_ {
    ScnFLOAT    x;
    ScnFLOAT    y;
} STScnPoint;

typedef struct STScnPointI_ {
    ScnSI32     x;
    ScnSI32     y;
} STScnPointI;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnPoint_h */
