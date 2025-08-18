//
//  ScnApiMetalSampler.m
//  ixtli-render
//
//  Created by Marcos Ortega on 18/8/25.
//

#include "ScnApiMetalSampler.h"


void ScnApiMetalSampler_free(void* pObj){
    STScnApiMetalSampler* obj = (STScnApiMetalSampler*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        if(obj->smplr != nil){
            [obj->smplr release];
            obj->smplr = nil;
        }
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

STScnGpuSamplerCfg ScnApiMetalSampler_getCfg(void* pObj){
    STScnGpuSamplerCfg r = STScnGpuSamplerCfg_Zero;
    STScnApiMetalSampler* obj = (STScnApiMetalSampler*)pObj;
    if(obj != NULL ){
        r = obj->cfg;
    }
    return r;
}
