//
//  ScnNode2dProps.h
//  ixtli-render
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnNode2dProps_h
#define ScnNode2dProps_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/type/ScnColor.h"
#include "ixrender/scene/ScnTransform2D.h"
#include "ixrender/gpu/ScnGpuModelProps2D.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnNode2dProps

#define STScnNode2dProps_Zero        { STScnColor8_Zero, STScnTransform2D_Zero }
#define STScnNode2dProps_Identity    { STScnColor8_255, STScnTransform2D_Identity }

typedef struct STScnNode2dProps {
    STScnColor8         c8;     //color
    STScnTransform2D    tform;  //transform
} STScnNode2dProps;

SC_INLN STScnGpuModelProps2D ScnNode2dProps_toGpuTransform(const STScnNode2dProps* const obj){
    return (STScnGpuModelProps2D){
        obj->c8
        , ScnMatrix2D_fromTransforms((const STScnPoint2D){ obj->tform.tx, obj->tform.ty }, DEG_2_RAD(obj->tform.deg), (const STScnSize2D){ obj->tform.sx, obj->tform.sy })
    };
}

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnNode2dProps_h */
