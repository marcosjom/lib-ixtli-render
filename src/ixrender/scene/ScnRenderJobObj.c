//
//  ScnRenderJobObj.c
//  ixtli-render
//
//  Created by Marcos Ortega on 22/8/25.
//

#include "ixrender/scene/ScnRenderJobObj.h"


//STScnRenderJobObj

void ScnRenderJobObj_init(STScnRenderJobObj* obj){
    ScnMemory_setZeroSt(*obj);
}

void ScnRenderJobObj_destroy(STScnRenderJobObj* obj){
    switch(obj->type){
        case ENScnRenderJobObjType_Unknown:
            SCN_ASSERT(obj->objRef.ptr == NULL);
            break;
        case ENScnRenderJobObjType_Buff:
        case ENScnRenderJobObjType_Framebuff:
        case ENScnRenderJobObjType_Vertexbuff:
        case ENScnRenderJobObjType_Texture:
            ScnObjRef_releaseAndNull(&obj->objRef);
            break;
        default:
            SCN_ASSERT(ScnFALSE); //missing implementation
            break;
    }
}

ScnSI32 ScnCompare_ScnRenderJobObj(const void* data1, const void* data2, const ScnUI32 dataSz){
    SCN_ASSERT(dataSz == sizeof(STScnRenderJobObj))
    if(dataSz == sizeof(STScnRenderJobObj)){
        const STScnRenderJobObj* d1 = (STScnRenderJobObj*)data1;
        const STScnRenderJobObj* d2 = (STScnRenderJobObj*)data2;
        return (d1->objRef.ptr < d2->objRef.ptr ? -1 : d1->objRef.ptr > d2->objRef.ptr ? 1 : 0);
    }
    return -2;
}
