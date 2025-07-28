//
//  ScnTransform.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnTransform_h
#define ScnTransform_h

#include "ixrender/ixtli-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnRenderApiItf

#define STScnTransform_Zero     { 0.f, 0.f, 0.f, 0.f, 0.f }
#define STScnTransform_Identity { 1.f, 1.f, 0.f, 0.f, 0.f }

typedef struct STScnTransform_ {
    ScnFLOAT sx;   //scale-x
    ScnFLOAT sy;   //scale-y
    ScnFLOAT tx;   //traslate-x
    ScnFLOAT ty;   //traslate-y
    ScnFLOAT deg;  //rotation in degrees
} STScnTransform;


#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnTransform_h */
