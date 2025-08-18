//
//  ScnApiMetalVertexbuff.m
//  ixtli-render
//
//  Created by Marcos Ortega on 18/8/25.
//

#include "ScnApiMetalVertexbuff.h"


void ScnApiMetalVertexbuff_free(void* pObj){
    STScnApiMetalVertexbuff* obj = (STScnApiMetalVertexbuff*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        ScnGpuBuffer_releaseAndNull(&obj->vBuff);
        ScnGpuBuffer_releaseAndNull(&obj->idxBuff);
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

ScnBOOL ScnApiMetalVertexbuff_sync(void* pObj, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff){
    ScnBOOL r = ScnFALSE;
    STScnApiMetalVertexbuff* obj = (STScnApiMetalVertexbuff*)pObj;
    {
        obj->cfg = *cfg;
        ScnGpuBuffer_set(&obj->vBuff, vBuff);
        ScnGpuBuffer_set(&obj->idxBuff, idxBuff);
        //ToDo: implement
        r = ScnTRUE;
    }
    return r;
}

ScnBOOL ScnApiMetalVertexbuff_activate(void* pObj){
    ScnBOOL r = ScnFALSE;
    //STScnApiMetalVertexbuff* obj = (STScnApiMetalVertexbuff*)pObj;
    {
        //ToDo: implement
        r = ScnTRUE;
    }
    return r;
}

ScnBOOL ScnApiMetalVertexbuff_deactivate(void* pObj){
    ScnBOOL r = ScnFALSE;
    //STScnApiMetalVertexbuff* obj = (STScnApiMetalVertexbuff*)pObj;
    {
        //ToDo: implement
        r = ScnTRUE;
    }
    return r;
}
