//
//  ScnNodeProps.h
//  ixtli-render
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnNodeProps_h
#define ScnNodeProps_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/scene/ScnColor.h"
#include "ixrender/scene/ScnTransform.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnNodeProps

#define STScnNodeProps_Zero   { STScnColor8_Zero, STScnTransform_Zero }

typedef struct STScnNodeProps_ {
    STScnColor8     c8;     //color
    STScnTransform  tform;  //transform
} STScnNodeProps;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnNodeProps_h */
