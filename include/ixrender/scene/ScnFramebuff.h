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

//ScnFramebuffRef

SCN_REF_STRUCT_METHODS_DEC(ScnFramebuff)

//

ScnBOOL         ScnFramebuff_prepare(ScnFramebuffRef ref, ScnGpuDeviceRef gpuDev);

//binding

ScnBOOL         ScnFramebuff_bindToOSView(ScnFramebuffRef ref, void* mtkView);

STScnSize2DU    ScnFramebuff_getSize(ScnFramebuffRef ref, STScnRectU* dstViewport);
ScnBOOL         ScnFramebuff_syncSizeAndViewport(ScnFramebuffRef ref, const STScnSize2DU size, const STScnRectU viewport);

//gpu


ScnBOOL         ScnFramebuff_prepareCurrentRenderSlot(ScnFramebuffRef ref);
ScnBOOL         ScnFramebuff_moveToNextRenderSlot(ScnFramebuffRef ref);
ScnGpuFramebuffRef ScnFramebuff_getCurrentRenderSlot(ScnFramebuffRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnFramebuff_h */
