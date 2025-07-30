//
//  ScnFramebuff.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnFramebuff_h
#define ScnFramebuff_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/gpu/ScnGpuDevice.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnFramebuffRef

SCN_REF_STRUCT_METHODS_DEC(ScnFramebuff)

//

ScnBOOL ScnFramebuff_prepare(STScnFramebuffRef ref, STScnGpuDeviceRef gpuDev, const ScnUI32 ammRenderSlots);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnFramebuff_h */
