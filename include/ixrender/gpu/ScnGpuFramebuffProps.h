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

/** @struct STScnGpuFramebuffProps
 *  @brief Properties for the scene rendering over the framebuffer.
 *  @var STScnGpuFramebuffProps::viewport
 *  Framebuffer surface's viewport rectangle.
 *  @var STScnGpuFramebuffProps::ortho
 *  Scene orthogonal boundaries' box.
 */

#define STScnGpuFramebuffProps_Zero        { STScnRectU_Zero, STScnAABBox3d_Zero }

typedef struct STScnGpuFramebuffProps {
    STScnRectU      viewport;   //render viewport
    STScnAABBox3d   ortho;      //render scene's orthogonal box
} STScnGpuFramebuffProps;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuFramebuffProps_h */
