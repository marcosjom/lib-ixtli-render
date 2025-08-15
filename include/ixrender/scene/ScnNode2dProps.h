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
#include "ixrender/scene/ScnTransform2d.h"
#include "ixrender/gpu/ScnGpuModelProps2d.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnNode2dProps

#define STScnNode2dProps_Zero        { STScnColor8_Zero, STScnTransform2d_Zero }
#define STScnNode2dProps_Identity    { STScnColor8_255, STScnTransform2d_Identity }

typedef struct STScnNode2dProps {
    STScnColor8         c8;     //color
    STScnTransform2d    tform;  //transform
} STScnNode2dProps;

SC_INLN STScnGpuModelProps2d ScnNode2dProps_toGpuTransform(const STScnNode2dProps* const obj){
    return SCN_ST(STScnGpuModelProps2d,
        {
            obj->c8
            , ScnMatrix2D_fromTransforms(SCN_ST(STScnPoint2D, { obj->tform.tx, obj->tform.ty }), DEG_2_RAD(obj->tform.deg), SCN_ST(STScnSize2D, { obj->tform.sx, obj->tform.sy }))
            , STScnGpuModelProps2d_Extra_Zero
        }
    );
}

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnNode2dProps_h */
