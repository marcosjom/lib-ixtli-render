//
//  ScnModelProps.h
//  ixtli-render
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnModelProps_h
#define ScnModelProps_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/type/ScnColor.h"
#include "ixrender/scene/ScnTransform.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnModelProps

#define STScnModelProps_Zero        { STScnColor8_Zero, STScnTransform_Zero }
#define STScnModelProps_Identity    { STScnColor8_255, STScnTransform_Identity }

typedef struct STScnModelProps_ {
    STScnColor8     c8;     //color
    STScnTransform  tform;  //transform
} STScnModelProps;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnModelProps_h */
