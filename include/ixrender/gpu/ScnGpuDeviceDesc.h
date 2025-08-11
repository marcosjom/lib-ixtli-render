//
//  ScnGpuDeviceDesc.h
//  ixtli-render
//
//  Created by Marcos Ortega on 10/8/25.
//

#ifndef ScnGpuDeviceDesc_h
#define ScnGpuDeviceDesc_h

#include "ixrender/ixtli-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnGpuDeviceDesc

#define STScnGpuDeviceDesc_Zero   { ScnFALSE, 0, 0 }

typedef struct STScnGpuDeviceDesc {
    ScnBOOL     isTexFmtInfoRequired;   //the device requires texture format info, if TRUE 'STScnGpuModelProps2d.texs' will be populated
    ScnUI32     offsetsAlign;           //buffers offsets aligment (example 32 bytes)
    ScnUI32     memBlockAlign;          //buffer memory copy alignment (example 256 bytes)
} STScnGpuDeviceDesc;


#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuDeviceDesc_h */
