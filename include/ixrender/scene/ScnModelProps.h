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
#include "ixrender/gpu/ScnGpuTransform.h"

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

SC_INLN STScnGpuTransform ScnModelProps_toGpuTransform(const STScnModelProps* const obj){
    return (STScnGpuTransform){
        obj->c8
        , ScnMatrix_fromTransforms((const STScnPoint){ obj->tform.tx, obj->tform.ty }, DEG_2_RAD(obj->tform.deg), (const STScnSize){ obj->tform.sx, obj->tform.sy })
    };
}

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnModelProps_h */
