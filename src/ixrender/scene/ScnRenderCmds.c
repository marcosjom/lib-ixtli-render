//
//  ScnRenderCmds.c
//  ixtli-render
//
//  Created by Marcos Ortega on 10/8/25.
//

#include "ixrender/scene/ScnRenderCmds.h"

//STScnRenderCmds

void ScnRenderCmds_init(ScnContextRef ctx, STScnRenderCmds* obj){
    ScnMemory_setZeroSt(*obj);
    ScnContext_set(&obj->ctx, ctx);
    //
    ScnArray_init(obj->ctx, &obj->stack, 0, 256, STScnRenderFbuffState)
    ScnArray_init(obj->ctx, &obj->cmds, 0, 256, STScnRenderCmd)
    ScnArraySorted_init(obj->ctx, &obj->objs, 0, 32, STScnRenderJobObj, ScnCompare_ScnRenderJobObj);
}

void ScnRenderCmds_destroy(STScnRenderCmds* obj){
    //objs
    {
        ScnArraySorted_foreach(&obj->objs, STScnRenderJobObj, o,
            ScnRenderJobObj_destroy(o);
        );
        obj->objs.use = 0;
        ScnArraySorted_destroy(obj->ctx, &obj->objs);
    }
    //stack
    {
        ScnArray_foreach(&obj->stack, STScnRenderFbuffState, s,
            ScnRenderFbuffState_destroy(s);
        );
        obj->stack.use = 0;
        obj->stackUse = 0;
        ScnArray_destroy(obj->ctx, &obj->stack);
    }
    //cmds
    {
        ScnArray_destroy(obj->ctx, &obj->cmds);
    }
    //
    if(!ScnMemElastic_isNull(obj->mPropsScns) && ScnMemElastic_hasPtrs(obj->mPropsScns)){
        SCN_PRINTF_WARNING("ScnRenderCmds_destroy::mPropsScns has data (STScnRenderCmds should be reused in normal operation).\n");
        ScnMemElastic_clear(obj->mPropsScns);
    }
    if(!ScnMemElastic_isNull(obj->mPropsMdls) && ScnMemElastic_hasPtrs(obj->mPropsMdls)){
        SCN_PRINTF_WARNING("ScnRenderCmds_destroy::mPropsMdls has data (STScnRenderCmds should be reused in normal operation).\n");
        ScnMemElastic_clear(obj->mPropsMdls);
    }
    ScnMemElastic_releaseAndNull(&obj->mPropsScns); //buffer with viewport properties
    ScnMemElastic_releaseAndNull(&obj->mPropsMdls); //buffer with viewport properties
    //
    ScnContext_releaseAndNull(&obj->ctx);
}

ScnBOOL ScnRenderCmds_prepare(STScnRenderCmds* obj, const STScnGpuDeviceDesc* gpuDevDesc, const ScnUI32 propsScnsPerBlock, const ScnUI32 propsMdlsPerBlock){
    ScnBOOL r = ScnFALSE;
    if(gpuDevDesc == NULL || gpuDevDesc->offsetsAlign <= 0 || gpuDevDesc->memBlockAlign <= 0 || propsScnsPerBlock <= 0 || propsMdlsPerBlock <= 0){
        return ScnFALSE;
    }
    //
    r = ScnTRUE;
    const ScnBOOL gpuDescChanged = (obj->gpuDevDesc.offsetsAlign != gpuDevDesc->offsetsAlign || obj->gpuDevDesc.offsetsAlign != gpuDevDesc->offsetsAlign ? ScnTRUE : ScnFALSE);
    //
    if(r && (ScnMemElastic_isNull(obj->mPropsScns) || gpuDescChanged)){
        STScnMemElasticCfg mPropsScnsCfg = STScnMemElasticCfg_Zero;
        ScnMemElasticRef mPropsScns = ScnMemElastic_alloc(obj->ctx);
        STScnMemElasticCfg* cfg     = &mPropsScnsCfg;
        const ScnUI32 itmsPerBlock  = propsScnsPerBlock;
        const ScnUI32 itmSz         = sizeof(STScnGpuFramebuffProps);
        cfg->idxZeroIsValid         = ScnTRUE;
        cfg->idxsAlign              = (itmSz + gpuDevDesc->offsetsAlign - 1) / gpuDevDesc->offsetsAlign * gpuDevDesc->offsetsAlign;
        cfg->sizeAlign              = gpuDevDesc->memBlockAlign;
        cfg->sizeInitial            = 0;
        //calculate sizePerBlock
        {
            ScnUI32 idxExtra        = 0;
            cfg->sizePerBlock       = ((itmsPerBlock * cfg->idxsAlign) + cfg->sizeAlign - 1) / cfg->sizeAlign * cfg->sizeAlign;
            idxExtra                = cfg->sizePerBlock % cfg->idxsAlign;
            if(idxExtra > 0){
                cfg->sizePerBlock   *= (cfg->idxsAlign - idxExtra);
            }
        }
        if(ScnMemElastic_isNull(mPropsScns) || !ScnMemElastic_prepare(mPropsScns, &mPropsScnsCfg, NULL)){
            printf("ERROR, ScnRenderCmds_prepare::ScnMemElastic_prepare(mPropsScns) failed.\n");
            r = ScnFALSE;
        } else {
            ScnMemElastic_set(&obj->mPropsScns, mPropsScns);
        }
        ScnMemElastic_releaseAndNull(&mPropsScns);
    }
    if(r && (ScnMemElastic_isNull(obj->mPropsMdls) || gpuDescChanged)){
        STScnMemElasticCfg mPropsMdlsCfg = STScnMemElasticCfg_Zero;
        ScnMemElasticRef mPropsMdls = ScnMemElastic_alloc(obj->ctx);
        STScnMemElasticCfg* cfg     = &mPropsMdlsCfg;
        const ScnUI32 itmsPerBlock  = propsMdlsPerBlock;
        const ScnUI32 itmSz         = sizeof(STScnGpuModelProps2d);
        cfg->idxZeroIsValid         = ScnTRUE;
        cfg->idxsAlign              = (itmSz + gpuDevDesc->offsetsAlign - 1) / gpuDevDesc->offsetsAlign * gpuDevDesc->offsetsAlign;
        cfg->sizeAlign              = gpuDevDesc->memBlockAlign;
        cfg->sizeInitial            = 0;
        //calculate sizePerBlock
        {
            ScnUI32 idxExtra        = 0;
            cfg->sizePerBlock       = ((itmsPerBlock * cfg->idxsAlign) + cfg->sizeAlign - 1) / cfg->sizeAlign * cfg->sizeAlign;
            idxExtra                = cfg->sizePerBlock % cfg->idxsAlign;
            if(idxExtra > 0){
                cfg->sizePerBlock   *= (cfg->idxsAlign - idxExtra);
            }
        }
        if(ScnMemElastic_isNull(mPropsMdls) || !ScnMemElastic_prepare(mPropsMdls, &mPropsMdlsCfg, NULL)){
            printf("ERROR, ScnRenderCmds_prepare::ScnMemElastic_prepare(mPropsMdls) failed.\n");
            r = ScnFALSE;
        } else {
            ScnMemElastic_set(&obj->mPropsMdls, mPropsMdls);
        }
        ScnMemElastic_releaseAndNull(&mPropsMdls);
    }
    if(r){
        if(gpuDescChanged){
            obj->gpuDevDesc = *gpuDevDesc;
        }
        r = obj->isPrepared = ScnTRUE;
    }
    return r;
}

void ScnRenderCmds_reset(STScnRenderCmds* obj){
    //stack
    {
        obj->stackUse = 0;
    }
    //cmds
    {
        ScnArray_empty(&obj->cmds);
    }
    //objs
    {
        ScnArraySorted_foreach(&obj->objs, STScnRenderJobObj, o,
            ScnRenderJobObj_destroy(o);
         );
        obj->objs.use = 0;
    }
    //buffs
    {
        if(!ScnMemElastic_isNull(obj->mPropsScns)){
            ScnMemElastic_clear(obj->mPropsScns);
        }
        if(!ScnMemElastic_isNull(obj->mPropsMdls)){
            ScnMemElastic_clear(obj->mPropsMdls);
        }
    }
}

ScnBOOL ScnRenderCmds_add(STScnRenderCmds* obj, const STScnRenderCmd* const cmd){
    return ScnArray_addPtr(obj->ctx, &obj->cmds, cmd, STScnRenderCmd) != NULL ? ScnTRUE : ScnFALSE;
}

ScnBOOL ScnRenderCmds_addUsedObj(STScnRenderCmds* obj, const ENScnRenderJobObjType type, ScnObjRef* objRef, ScnBOOL* optDstIsFirstUse){
    SCN_ASSERT(!ScnObjRef_isNull(*objRef)) //program logic error (validate becfore reaching this point)
    ScnBOOL r = ScnFALSE, isFirstUse = ScnFALSE;
    STScnRenderJobObj usedObj;
    usedObj.type    = type;
    usedObj.objRef  = *objRef;
    const ScnSI32 iFnd = ScnArraySorted_indexOf(&obj->objs, &usedObj);
    if(iFnd >= 0){
        //already added;
        r = ScnTRUE;
    } else {
        //first time used, add to array
        ScnRenderJobObj_init(&usedObj);
        usedObj.type = type;
        ScnObjRef_set(&usedObj.objRef, *objRef);
        if(NULL == ScnArraySorted_addPtr(obj->ctx, &obj->objs, &usedObj, STScnRenderJobObj)){
            printf("ERROR, ScnRenderCmds_addUsedObj::ScnArraySorted_addPtr(usedObj) failed.\n");
            ScnRenderJobObj_destroy(&usedObj);
        } else {
            r = ScnTRUE;
            isFirstUse = ScnTRUE;
        }
    }
    if(optDstIsFirstUse != NULL) *optDstIsFirstUse = isFirstUse;
    return r;
}


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

ScnBOOL ScnCompare_ScnRenderJobObj(const ENScnCompareMode mode, const void* data1, const void* data2, const ScnUI32 dataSz){
    SCN_ASSERT(dataSz == sizeof(STScnRenderJobObj))
    if(dataSz == sizeof(STScnRenderJobObj)){
        const STScnRenderJobObj* d1 = (STScnRenderJobObj*)data1;
        const STScnRenderJobObj* d2 = (STScnRenderJobObj*)data2;
        switch (mode) {
            case ENScnCompareMode_Equal: return d1->objRef.ptr == d2->objRef.ptr;
            case ENScnCompareMode_Lower: return d1->objRef.ptr < d2->objRef.ptr;
            case ENScnCompareMode_LowerOrEqual: return d1->objRef.ptr <= d2->objRef.ptr;
            case ENScnCompareMode_Greater: return d1->objRef.ptr > d2->objRef.ptr;
            case ENScnCompareMode_GreaterOrEqual: return d1->objRef.ptr >= d2->objRef.ptr;
            default: SCN_ASSERT(ScnFALSE) break;
        }
    }
    return ScnFALSE;
}

