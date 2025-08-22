//
//  ScnRenderFbuffState.c
//  ixtli-render
//
//  Created by Marcos Ortega on 22/8/25.
//

#include "ixrender/scene/ScnRenderFbuffState.h"


//STScnRenderFbuffState

void ScnRenderFbuffState_init(ScnContextRef ctx, STScnRenderFbuffState* obj){
    ScnMemory_setZeroSt(*obj);
    ScnContext_set(&obj->ctx, ctx);
    //stacks
    {
        ScnArray_init(obj->ctx, &obj->stacks.props, 8, 32, STScnRenderFbuffProps);
        ScnArray_init(obj->ctx, &obj->stacks.transforms, 256, 256, STScnGpuModelProps2d);
    }
}

void ScnRenderFbuffState_destroy(STScnRenderFbuffState* obj){
    //stacks
    {
        ScnArray_destroy(obj->ctx, &obj->stacks.props);
        ScnArray_destroy(obj->ctx, &obj->stacks.transforms);
    }
    //
    ScnContext_releaseAndNull(&obj->ctx);
}
