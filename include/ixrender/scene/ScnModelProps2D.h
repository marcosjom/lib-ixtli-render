//
//  ScnModelProps2D.h
//  ixtli-render
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnModelProps2D_h
#define ScnModelProps2D_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/type/ScnColor.h"
#include "ixrender/scene/ScnTransform2D.h"
#include "ixrender/gpu/ScnGpuModelProps2D.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnModelProps2D

#define STScnModelProps2D_Zero        { STScnColor8_Zero, STScnTransform2D_Zero }
#define STScnModelProps2D_Identity    { STScnColor8_255, STScnTransform2D_Identity }

typedef struct STScnModelProps2D {
    STScnColor8         c8;     //color
    STScnTransform2D    tform;  //transform
} STScnModelProps2D;

SC_INLN STScnGpuModelProps2D ScnModelProps2D_toGpuTransform(const STScnModelProps2D* const obj){
    return (STScnGpuModelProps2D){
        obj->c8
        , ScnMatrix2D_fromTransforms((const STScnPoint2D){ obj->tform.tx, obj->tform.ty }, DEG_2_RAD(obj->tform.deg), (const STScnSize2D){ obj->tform.sx, obj->tform.sy })
    };
}

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnModelProps2D_h */
