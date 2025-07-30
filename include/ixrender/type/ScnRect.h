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

#define STScnRect_Zero { 0.f, 0.f, 0.f, 0.f }

typedef struct STScnRect_ {
    ScnFLOAT    x;
    ScnFLOAT    y;
    ScnFLOAT    width;
    ScnFLOAT    height;
} STScnRect;

#define STScnRectI_Zero { 0, 0, 0, 0 }

typedef struct STScnRectI_ {
    ScnSI32     x;
    ScnSI32     y;
    ScnSI32     width;
    ScnSI32     eight;
} STScnRectI;

#define STScnRectU_Zero { 0u, 0u, 0u, 0u }

typedef struct STScnRectU_ {
    ScnUI32     x;
    ScnUI32     y;
    ScnUI32     width;
    ScnUI32     eight;
} STScnRectU;

#define STScnRectI16_Zero { 0, 0, 0, 0 }

typedef struct STScnRectI16_ {
    ScnSI16     x;
    ScnSI16     y;
    ScnSI16     width;
    ScnSI16     height;
} STScnRectI16;

#define STScnRectU16_Zero { 0u, 0u, 0u, 0u }

typedef struct STScnRectU16_ {
    ScnUI16     x;
    ScnUI16     y;
    ScnUI16     width;
    ScnUI16     height;
} STScnRectU16;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRect_h */
