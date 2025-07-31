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

#ifdef __cplusplus
extern "C" {
#endif

//STScnModelProps2D

#define STScnGpuFramebufferProps_Zero        { STScnSize2DU_Zero, STScnRectU_Zero }

typedef struct STScnGpuFramebufferProps_ {
    STScnSize2DU      size;       //buffe 2D size
    STScnRectU      viewport;   //render viewport
} STScnGpuFramebufferProps;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuFramebuffProps_h */
