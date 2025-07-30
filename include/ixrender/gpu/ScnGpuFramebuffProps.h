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

//STScnModelProps

#define STScnGpuFramebufferProps_Zero        { STScnSizeU_Zero, STScnRectU_Zero }

typedef struct STScnGpuFramebufferProps_ {
    STScnSizeU      size;       //buffe 2D size
    STScnRectU      viewport;   //render viewport
} STScnGpuFramebufferProps;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuFramebuffProps_h */
