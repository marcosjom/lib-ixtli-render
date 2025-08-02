//
//  ScnGpuFramebuffProps.h
//  ixtli-render
//
//  Created by Marcos Ortega on 30/7/25.
//

#ifndef ScnGpuFramebuffProps_h
#define ScnGpuFramebuffProps_h

#include "ixrender/type/ScnSize.h"
#include "ixrender/type/ScnRect.h"
#include "ixrender/type/ScnAABBox.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnNode2dProps

#define STScnGpuFramebuffProps_Zero        { STScnSize2DU_Zero, STScnRectU_Zero, STScnAABBox3d_Zero }

typedef struct STScnGpuFramebuffProps {
    STScnSize2DU    size;       //buffer size
    STScnRectU      viewport;   //render viewport
    STScnAABBox3d   ortho;      //render orthogonal box
} STScnGpuFramebuffProps;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuFramebuffProps_h */
