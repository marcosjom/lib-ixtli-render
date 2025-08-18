//
//  ScnApiMetalFramebuff.m
//  ixtli-render
//
//  Created by Marcos Ortega on 18/8/25.
//

#include "ScnApiMetalFramebuff.h"

//frameBuffer (view)

void ScnApiMetalFramebuff_view_free(void* pObj){
    STScnApiMetalFramebuffView* obj = (STScnApiMetalFramebuffView*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        //
        STScnApiMetalRenderStates_destroy(&obj->rndrShaders);
        //
        if(obj->mtkView != nil){
            [obj->mtkView release];
            obj->mtkView = nil;
        }
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

STScnSize2DU ScnApiMetalFramebuff_view_getSize(void* pObj){
    STScnApiMetalFramebuffView* obj = (STScnApiMetalFramebuffView*)pObj;
    return obj->size;
}

ScnBOOL ScnApiMetalFramebuff_view_syncSize(void* pObj, const STScnSize2DU size){
    ScnBOOL r = ScnFALSE;
    STScnApiMetalFramebuffView* obj = (STScnApiMetalFramebuffView*)pObj;
    if(obj->mtkView != nil){
        //const CGSize sz = obj->mtkView.drawableSize;
        //obj->size.width = sz.width;
        //obj->size.height = sz.height;
        obj->size = size;
        r = ScnTRUE;
    }
    return r;
}

STScnGpuFramebuffProps ScnApiMetalFramebuff_view_getProps(void* pObj){
    STScnApiMetalFramebuffView* obj = (STScnApiMetalFramebuffView*)pObj;
    return obj->props;
}

ScnBOOL ScnApiMetalFramebuff_view_setProps(void* pObj, const STScnGpuFramebuffProps* const props){
    ScnBOOL r = ScnFALSE;
    STScnApiMetalFramebuffView* obj = (STScnApiMetalFramebuffView*)pObj;
    if(obj->mtkView != nil && props != NULL){
        obj->props = *props;
        r = ScnTRUE;
    }
    return r;
}
