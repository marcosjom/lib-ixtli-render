//
//  ScnTransform2d.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnTransform2d_h
#define ScnTransform2d_h

#include "ixrender/ixtli-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnRenderApiItf

#define STScnTransform2d_Zero     { 0.f, 0.f, 0.f, 0.f, 0.f }
#define STScnTransform2d_Identity { 1.f, 1.f, 0.f, 0.f, 0.f }

typedef struct STScnTransform2d {
    ScnFLOAT sx;   //scale-x
    ScnFLOAT sy;   //scale-y
    ScnFLOAT tx;   //traslate-x
    ScnFLOAT ty;   //traslate-y
    ScnFLOAT deg;  //rotation in degrees
} STScnTransform2d;


#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnTransform2d_h */
