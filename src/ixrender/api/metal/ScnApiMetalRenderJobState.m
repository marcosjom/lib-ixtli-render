//
//  ScnApiMetalRenderJobState.m
//  ixtli-render
//
//  Created by Marcos Ortega on 18/8/25.
//

#include "ScnApiMetalRenderJobState.h"


// STScnApiMetalRenderJobState

void ScnApiMetalRenderJobState_init(STScnApiMetalRenderJobState* obj){
    ScnMemory_setZeroSt(*obj);
}

void ScnApiMetalRenderJobState_destroy(STScnApiMetalRenderJobState* obj){
    if(obj->rndrDesc != nil){ [obj->rndrDesc release]; obj->rndrDesc = nil; }
    if(obj->rndrEnc != nil){ [obj->rndrEnc release]; obj->rndrEnc = nil; }
}

void ScnApiMetalRenderJobState_reset(STScnApiMetalRenderJobState* obj){
    if(obj->rndrDesc != nil){ [obj->rndrDesc release]; obj->rndrDesc = nil; }
    if(obj->rndrEnc != nil){ [obj->rndrEnc release]; obj->rndrEnc = nil; }
    ScnMemory_setZeroSt(*obj);
}
