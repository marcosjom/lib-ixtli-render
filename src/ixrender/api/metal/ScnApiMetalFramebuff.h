//
//  ScnApiMetalFramebuff.h
//  ixtli-render
//
//  Created by Marcos Ortega on 18/8/25.
//

#ifndef ScnApiMetalFramebuff_h
#define ScnApiMetalFramebuff_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnContext.h"
#include "ixrender/type/ScnSize.h"
#include "ixrender/gpu/ScnGpuFramebuff.h"
#include "ixrender/api/ScnApiItf.h"
//
#include "ScnApiMetalRenderStates.h"
#include "ScnApiMetalBuffer.h"
//
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import <TargetConditionals.h>  //for TARGET_OS_* macros

#ifdef __cplusplus
extern "C" {
#endif

//STScnApiMetalFramebuffView

typedef struct STScnApiMetalFramebuffView {
    ScnContextRef           ctx;
    MTKView*                mtkView;
    STScnSize2DU            size;
    STScnGpuFramebuffProps  props;
    STScnApiItf             itf;
    STScnApiMetalRenderStates rndrShaders; //shaders
    //cur (state while sending commands)
    struct {
        //verts
        struct {
            ENScnVertexType type;
            STScnApiMetalBuffer* buff;
            STScnApiMetalBuffer* idxs;
        } verts;
    } cur;
} STScnApiMetalFramebuffView;

void            ScnApiMetalFramebuff_view_free(void* data);
STScnSize2DU    ScnApiMetalFramebuff_view_getSize(void* pObj);
ScnBOOL         ScnApiMetalFramebuff_view_syncSize(void* pObj, const STScnSize2DU size);
STScnGpuFramebuffProps ScnApiMetalFramebuff_view_getProps(void* data);
ScnBOOL         ScnApiMetalFramebuff_view_setProps(void* data, const STScnGpuFramebuffProps* const props);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiMetalFramebuff_h */
