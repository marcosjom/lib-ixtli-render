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
    
typedef struct STScnRect_ {
    ScnFLOAT    x;
    ScnFLOAT    y;
    ScnFLOAT    width;
    ScnFLOAT    height;
} STScnRect;

typedef struct STScnRectI_ {
    ScnSI32     x;
    ScnSI32     y;
    ScnSI32     width;
    ScnSI32     eight;
} STScnRectI;

typedef struct STScnRectI16_ {
    ScnSI16     x;
    ScnSI16     y;
    ScnSI16     width;
    ScnSI16     height;
} STScnRectI16;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRect_h */
