//
//  ScnRender.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/ScnRender.h"
#include "ixrender/core/ScnArray.h"
#include "ixrender/core/ScnArraySorted.h"
#include "ixrender/gpu/ScnGpuDataType.h"
#include "ixrender/gpu/ScnGpuFramebuffProps.h"
#include "ixrender/gpu/ScnGpuTransform.h"

//STScnRenderFbuffState

typedef struct STScnRenderFbuffState_ {
    STScnContextRef         ctx;
    STScnAbsPtr             props;
    ScnUI32                 stackUse;           //effective use of the stack-array (the array is reused between renders)
    ScnArrayStruct(stack, STScnGpuTransform);   //stack of transformations
} STScnRenderFbuffState;

void ScnRenderFbuffState_init(STScnContextRef ctx, STScnRenderFbuffState* obj);
void ScnRenderFbuffState_destroy(STScnRenderFbuffState* obj);
//
ScnBOOL ScnRenderFbuffState_addTransform(STScnRenderFbuffState* obj, const STScnGpuTransform* const t);

//STScnRenderOpq

typedef struct STScnRenderOpq_ {
    STScnContextRef         ctx;
    STScnMutexRef           mutex;
    ScnUI32                 ammRenderSlots;
    //api
    struct {
        STScnApiItf         itf;
        void*               itfParam;
    } api;
    STScnGpuDeviceRef       gpuDev;
    STScnBufferRef          viewPropsBuff;  //buffer with viewport properties
    STScnBufferRef          nodesPropsBuff; //buffer with models
    STScnVertexbuffsRef     vbuffs;         //buffers with vertices and indices
    //job
    struct {
        ScnBOOL             isActive;       //jobStart() was called without a jobEnd()
        ScnUI32             stackUse;       //effective use of the stack-array (the array is reused between renders)
        ScnArrayStruct(stack, STScnRenderFbuffState);   //stack of framebuffers
        ScnArrayStruct(cmds, STScnRenderCmd);           //cmds sequence
    } job;
} STScnRenderOpq;

//

ScnSI32 ScnRender_getOpqSz(void){
    return (ScnSI32)sizeof(STScnRenderOpq);
}

void ScnRender_initZeroedOpq(STScnContextRef ctx, void* obj) {
    STScnRenderOpq* opq = (STScnRenderOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex = ScnContext_allocMutex(opq->ctx);
    //api
    {
        //
    }
    //gpuDev
    {
        //
    }
    //job
    {
        ScnArray_init(opq->ctx, &opq->job.stack, 256, 256, STScnRenderFbuffState)
        ScnArray_init(opq->ctx, &opq->job.cmds, 256, 256, STScnRenderCmd)
    }
}

void ScnRender_destroyOpq(void* obj){
    STScnRenderOpq* opq = (STScnRenderOpq*)obj;
    //job
    {
        if(opq->job.stack.arr != NULL){
            STScnRenderFbuffState* s = opq->job.stack.arr;
            const STScnRenderFbuffState* sAfterEnd = s + opq->job.stack.use;
            while(s < sAfterEnd){
                ScnRenderFbuffState_destroy(s);
                ++s;
            }
        }
        opq->job.stackUse = 0;
        ScnArray_destroy(opq->ctx, &opq->job.stack);
        ScnArray_destroy(opq->ctx, &opq->job.cmds);
    }
    //viewPropsBuff
    if(!ScnBuffer_isNull(opq->viewPropsBuff)){
        ScnBuffer_release(&opq->viewPropsBuff);
        ScnBuffer_null(&opq->viewPropsBuff);
    }
    //nodesPropsBuff
    if(!ScnBuffer_isNull(opq->nodesPropsBuff)){
        ScnBuffer_release(&opq->nodesPropsBuff);
        ScnBuffer_null(&opq->nodesPropsBuff);
    }
    //vbuffs
    if(!ScnVertexbuffs_isNull(opq->vbuffs)){
        ScnVertexbuffs_release(&opq->vbuffs);
        ScnVertexbuffs_null(&opq->vbuffs);
    }
    //gpuDev
    if(!ScnGpuDevice_isNull(opq->gpuDev)){
        ScnGpuDevice_release(&opq->gpuDev);
        ScnGpuDevice_null(&opq->gpuDev);
    }
    //api
    {
        ScnMemory_setZeroSt(opq->api.itf, STScnApiItf);
        opq->api.itfParam = NULL;
    }
    //
    if(!ScnMutex_isNull(opq->mutex)){
        ScnMutex_free(&opq->mutex);
        ScnMutex_null(&opq->mutex);
    }
    //
    if(!ScnContext_isNull(opq->ctx)){
        ScnContext_release(&opq->ctx);
        ScnContext_null(&opq->ctx);
    }
}

//prepare

STScnBufferRef      ScnRender_allocDynamicBuffLockedOpq_(STScnRenderOpq* opq, const ScnUI32 itmSz, const ScnUI32 itmsPerBlock, const ScnUI32 ammRenderSlots);
STScnVertexbuffsRef ScnRender_allocVertexbuffsLockedOpq_(STScnRenderOpq* opq, const ScnUI32 ammRenderSlots);

ScnBOOL ScnRender_prepare(STScnRenderRef ref, const STScnApiItf* itf, void* itfParam){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(itf != NULL){
        opq->api.itf = *itf;
        opq->api.itfParam = itfParam;
        r = ScnTRUE;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//Device

ScnBOOL ScnRender_openDevice(STScnRenderRef ref, const STScnGpuDeviceCfg* cfg, const ScnUI32 ammRenderSlots){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(ScnGpuDevice_isNull(opq->gpuDev) && opq->api.itf.allocDevice != NULL && opq->ammRenderSlots == 0 && ammRenderSlots > 0){
        STScnGpuDeviceRef gpuDev = (*opq->api.itf.allocDevice)(opq->ctx, cfg);
        if(!ScnGpuDevice_isNull(gpuDev)){
            ScnGpuDevice_set(&opq->gpuDev, gpuDev);
            //
            opq->ammRenderSlots = ammRenderSlots;
            //initial buffers
            {
                STScnBufferRef viewPropsBuff = STScnObjRef_Zero;
                STScnBufferRef nodesPropsBuff = STScnObjRef_Zero;
                STScnVertexbuffsRef vbuffs = STScnObjRef_Zero;
                //
                viewPropsBuff = ScnRender_allocDynamicBuffLockedOpq_(opq, sizeof(STScnGpuFramebufferProps), 32, ammRenderSlots);
                if(ScnBuffer_isNull(viewPropsBuff)){
                    printf("ERROR, ScnRender_allocDynamicBuffLockedOpq_(viewPropsBuff) failed.\n");
                } else {
                    nodesPropsBuff = ScnRender_allocDynamicBuffLockedOpq_(opq, sizeof(STScnGpuTransform), 128, ammRenderSlots);
                    if(ScnBuffer_isNull(nodesPropsBuff)){
                        printf("ERROR, ScnRender_allocDynamicBuffLockedOpq_(nodesPropsBuff) failed.\n");
                    } else {
                        vbuffs = ScnRender_allocVertexbuffsLockedOpq_(opq, ammRenderSlots);
                        if(ScnVertexbuffs_isNull(vbuffs)){
                            printf("ERROR, ScnRender_allocVertexbuffsLockedOpq_(vbuffs) failed.\n");
                        } else {
                            ScnBuffer_set(&opq->viewPropsBuff, viewPropsBuff);
                            ScnBuffer_set(&opq->nodesPropsBuff, nodesPropsBuff);
                            ScnVertexbuffs_set(&opq->vbuffs, vbuffs);
                            r = ScnTRUE;
                        }
                    }
                }
                if(!ScnBuffer_isNull(viewPropsBuff)){
                    ScnBuffer_release(&viewPropsBuff);
                    ScnBuffer_null(&viewPropsBuff);
                }
                if(!ScnBuffer_isNull(nodesPropsBuff)){
                    ScnBuffer_release(&nodesPropsBuff);
                    ScnBuffer_null(&nodesPropsBuff);
                }
                if(!ScnVertexbuffs_isNull(vbuffs)){
                    ScnVertexbuffs_release(&vbuffs);
                    ScnVertexbuffs_null(&vbuffs);
                }
            }
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRender_hasOpenDevice(STScnRenderRef ref){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return !ScnGpuDevice_isNull(opq->gpuDev);
}

STScnModelRef ScnRender_allocModel(STScnRenderRef ref){
    STScnModelRef r = STScnObjRef_Zero;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnVertexbuffs_isNull(opq->vbuffs)){
        r = ScnModel_alloc(opq->ctx);
        if(ScnModel_isNull(r)){
            printf("ERROR, ScnRender_allocModel::ScnModel_alloc failed.\n");
        } else if(!ScnModel_setVertexBuffs(r, opq->vbuffs)){
            printf("ERROR, ScnRender_allocModel::ScnModel_setVertexBuffs failed.\n");
            ScnModel_release(&r);
            ScnModel_null(&r);
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//Vertices

STScnVertexbuffsRef ScnRender_getDefaultVertexbuffs(STScnRenderRef ref){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->vbuffs;
}

STScnVertexbuffsRef ScnRender_allocVertexbuffs(STScnRenderRef ref){
    STScnVertexbuffsRef r = STScnObjRef_Zero;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->ammRenderSlots > 0){
        r = ScnRender_allocVertexbuffsLockedOpq_(opq, opq->ammRenderSlots);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

STScnBufferRef ScnRender_allocDynamicBuffLockedOpq_(STScnRenderOpq* opq, const ScnUI32 itmSz, const ScnUI32 itmsPerBlock, const ScnUI32 ammRenderSlots){
    STScnBufferRef r        = STScnObjRef_Zero;
    STScnGpuBufferCfg cfg   = STScnGpuBufferCfg_Zero;
    cfg.mem.idxsAlign       = itmSz;
    cfg.mem.sizeAlign       = 256;
    cfg.mem.sizeInitial     = 0;
    //calculate sizePerBlock
    {
        ScnUI32 idxExtra = 0;
        cfg.mem.sizePerBlock = ((itmsPerBlock * cfg.mem.idxsAlign) + cfg.mem.sizeAlign - 1) / cfg.mem.sizeAlign * cfg.mem.sizeAlign;
        idxExtra = cfg.mem.sizePerBlock % cfg.mem.idxsAlign;
        if(idxExtra > 0){
            cfg.mem.sizePerBlock *= (cfg.mem.idxsAlign - idxExtra);
        }
        //SCN_ASSERT(0 == (cfg.mem.sizePerBlock % cfg.mem.idxsAlign))
        //SCN_ASSERT(0 == (cfg.mem.sizePerBlock % cfg.mem.sizeAlign))
    }
    r = ScnBuffer_alloc(opq->ctx);
    if(ScnBuffer_isNull(r)){
        //error
        SCN_ASSERT(ScnFALSE)
    } else if(!ScnBuffer_prepare(r, opq->gpuDev, ammRenderSlots, &cfg)){
        //error
        SCN_ASSERT(ScnFALSE)
        ScnBuffer_release(&r);
        ScnBuffer_null(&r);
    }
    return r;
}

STScnVertexbuffsRef ScnRender_allocVertexbuffsLockedOpq_(STScnRenderOpq* opq, const ScnUI32 ammRenderSlots){
    STScnVertexbuffsRef rr = STScnObjRef_Zero;
    ScnBOOL r = ScnTRUE;
    STScnBufferRef vBuffs[ENScnVertexType_Count];
    STScnBufferRef iBuffs[ENScnVertexType_Count];
    STScnVertexbuffRef vbs[ENScnVertexType_Count];
    memset(vBuffs, 0, sizeof(vBuffs));
    memset(iBuffs, 0, sizeof(iBuffs));
    memset(vbs, 0, sizeof(vbs));
    //initial bufffers
    if(r){
        ScnSI32 i; for(i = 0; i < ENScnVertexType_Count; i++){
            ScnUI32 itmSz = 0, ammPerBock = 0;
            switch(i){
                case ENScnVertexType_Color:
                    itmSz = sizeof(STScnVertex);
                    ammPerBock = 256;
                    break;
                case ENScnVertexType_Tex:
                    itmSz = sizeof(STScnVertexTex);
                    ammPerBock = 1024;
                    break;
                case ENScnVertexType_Tex2:
                    itmSz = sizeof(STScnVertexTex2);
                    ammPerBock = 256;
                    break;
                case ENScnVertexType_Tex3:
                    itmSz = sizeof(STScnVertexTex3);
                    ammPerBock = 256;
                    break;
                default: SCN_ASSERT(ScnFALSE); r = ScnFALSE; break;
            }
            //vertex buffer
            if(r){
                vBuffs[i] = ScnRender_allocDynamicBuffLockedOpq_(opq, itmSz, ammPerBock, ammRenderSlots);
                if(ScnBuffer_isNull(vBuffs[i])){
                    //error
                    SCN_ASSERT(ScnFALSE)
                    r = ScnFALSE;
                }
            }
            //indices buffer
            if(r){
                itmSz = sizeof(STScnVertexIdx);
                ammPerBock = 2048;
                iBuffs[i] = ScnRender_allocDynamicBuffLockedOpq_(opq, itmSz, ammPerBock, ammRenderSlots);
                if(ScnBuffer_isNull(iBuffs[i])){
                    //error
                    SCN_ASSERT(ScnFALSE)
                    r = ScnFALSE;
                }
            }
        }
    }
    //initial vertexBuffers
    if(r){
        ScnSI32 i; for(i = 0; i < ENScnVertexType_Count; i++){
            STScnGpuVertexbuffCfg cfg = STScnGpuVertexbuffCfg_Zero;
            STScnBufferRef vertexBuff = vBuffs[i];
            STScnBufferRef idxsBuff = iBuffs[i];
            //size
            switch(i){
                case ENScnVertexType_Tex3: cfg.szPerRecord = sizeof(STScnVertexTex3); break;
                case ENScnVertexType_Tex2: cfg.szPerRecord = sizeof(STScnVertexTex2); break;
                case ENScnVertexType_Tex: cfg.szPerRecord = sizeof(STScnVertexTex); break;
                case ENScnVertexType_Color: cfg.szPerRecord = sizeof(STScnVertex); break;
                default: SCN_ASSERT(ScnFALSE) r = ScnFALSE; break;
            }
            //SCN_ASSERT((cfg.szPerRecord % 4) == 0)
            //elems
            switch(i){
                case ENScnVertexType_Tex3:
                    cfg.texCoords[2].amm    = 2;
                    cfg.texCoords[2].type   = ENScnGpuDataType_FLOAT32;
                    cfg.texCoords[2].offset = ScnVertexTex3_IDX_tex3_x;
                case ENScnVertexType_Tex2:
                    cfg.texCoords[1].amm    = 2;
                    cfg.texCoords[1].type   = ENScnGpuDataType_FLOAT32;
                    cfg.texCoords[1].offset = ScnVertexTex2_IDX_tex2_x;
                case ENScnVertexType_Tex:
                    cfg.texCoords[0].amm    = 2;
                    cfg.texCoords[0].type   = ENScnGpuDataType_FLOAT32;
                    cfg.texCoords[0].offset = ScnVertexTex_IDX_tex_x;
                default:
                    //color
                    cfg.color.amm           = 4;
                    cfg.color.type          = ENScnGpuDataType_UI8;
                    cfg.color.offset        = ScnVertex_IDX_color;
                    //coord
                    cfg.coord.amm           = 2;
                    cfg.coord.type          = ENScnGpuDataType_FLOAT32;
                    cfg.coord.offset        = ScnVertex_IDX_x;
                    //indices
                    if(!ScnBuffer_isNull(idxsBuff)){
                        cfg.indices.amm     = 1;
                        cfg.indices.type    = ENScnGpuDataType_UI32;
                        cfg.indices.offset  = 0;
                    }
                    break;
            }
            if(r){
                vbs[i] = ScnVertexbuff_alloc(opq->ctx);
                if(!ScnVertexbuff_prepare(vbs[i], opq->gpuDev, ammRenderSlots, &cfg, vertexBuff, idxsBuff)){
                    //error
                    SCN_ASSERT(ScnFALSE)
                    r = ScnFALSE;
                }
            }
        }
        //
        if(r){
            STScnVertexbuffsRef vbObj = ScnVertexbuffs_alloc(opq->ctx);
            if(ScnVertexbuffs_prepare(vbObj, vbs, sizeof(vbs) / sizeof(vbs[0]))){
                ScnVertexbuffs_set(&rr, vbObj);
            } else {
                r = ScnFALSE;
            }
            ScnVertexbuffs_release(&vbObj);
            ScnVertexbuffs_null(&vbObj);
        }
    }
    //release
    {
        //vbs
        {
            STScnVertexbuffRef* b = vbs;
            const STScnVertexbuffRef* bAfterEnd = b + (sizeof(vbs) / sizeof(vbs[0]));
            while(b < bAfterEnd){
                ScnVertexbuff_release(b);
                ++b;
            }
        }
        //vBuffs
        {
            STScnBufferRef* b = vBuffs;
            const STScnBufferRef* bAfterEnd = b + (sizeof(vBuffs) / sizeof(vBuffs[0]));
            while(b < bAfterEnd){
                ScnBuffer_release(b);
                ++b;
            }
        }
        //iBuffs
        {
            STScnBufferRef* b = iBuffs;
            const STScnBufferRef* bAfterEnd = b + (sizeof(iBuffs) / sizeof(iBuffs[0]));
            while(b < bAfterEnd){
                ScnBuffer_release(b);
                ++b;
            }
        }
    }
    return rr;
}

//Buffers

STScnBufferRef ScnRender_allocBuffer(STScnRenderRef ref, const STScnGpuBufferCfg* cfg){ //allocates a new buffer
    STScnBufferRef r = STScnObjRef_Zero;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(cfg != NULL && opq->ammRenderSlots > 0){
        STScnBufferRef b = ScnBuffer_alloc(opq->ctx);
        if(!ScnBuffer_prepare(b, opq->gpuDev, opq->ammRenderSlots, cfg)){
            //error
            SCN_ASSERT(ScnFALSE)
        } else {
            ScnBuffer_set(&r, b);
        }
        ScnBuffer_release(&b);
        ScnBuffer_null(&b);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//job
    
ScnBOOL ScnRender_jobStart(STScnRenderRef ref){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->job.isActive){
        printf("ERROR, ScnRender_jobStart:: already active.\n");
    } else {
        r = ScnTRUE;
        //empty viewPropsBuff
        if(r && (ScnBuffer_isNull(opq->viewPropsBuff) || !ScnBuffer_clear(opq->viewPropsBuff) || !ScnBuffer_invalidateAll(opq->viewPropsBuff))){
            printf("ERROR, ScnRender_jobStart::ScnBuffer_clear(viewPropsBuff) failed.\n");
            r = ScnFALSE;
        }
        //empty nodesPropsBuff
        if(r && (ScnBuffer_isNull(opq->nodesPropsBuff) || !ScnBuffer_clear(opq->nodesPropsBuff) || !ScnBuffer_invalidateAll(opq->nodesPropsBuff))){
            printf("ERROR, ScnRender_jobStart::ScnBuffer_clear(nodesPropsBuff) failed.\n");
            r = ScnFALSE;
        }
        //empty arrays
        if(r){
            opq->job.stackUse = 0;
            ScnArray_empty(&opq->job.cmds);
            //
            opq->job.isActive = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}
    
ScnBOOL ScnRender_jobEnd(STScnRenderRef ref){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!opq->job.isActive){
        printf("ERROR, ScnRender_jobEnd:: not active.\n");
    } else {
        //ToDO: implement
        opq->job.isActive= ScnFALSE;
        r = ScnTRUE;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//job framebuffers

ScnBOOL ScnRender_jobFramebuffPush(STScnRenderRef ref, STScnFramebuffRef fbuff){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!opq->job.isActive){
        printf("ERROR, ScnRender_jobFramebuffPush:: not active.\n");
    } else if(!ScnBuffer_isNull(opq->viewPropsBuff)){
        STScnRenderFbuffState* f = NULL;
        if(opq->job.stackUse < opq->job.stack.use){
            //reuse record
            f = &opq->job.stack.arr[opq->job.stackUse];
        } else {
            STScnRenderFbuffState ff;
            ScnRenderFbuffState_init(opq->ctx, &ff);
            f = ScnArray_addPtr(opq->ctx, &opq->job.stack, &ff, STScnRenderFbuffState);
            if(f == NULL){
                printf("ERROR, ScnRender_jobFramebuffPush::ScnArray_addPtr failed.\n");
                ScnRenderFbuffState_destroy(&ff);
            }
        }
        //set initial state
        if(f != NULL){
            STScnGpuTransform t = STScnGpuTransform_Identity;
            f->stackUse = 0;
            f->props    = ScnBuffer_malloc(opq->viewPropsBuff, sizeof(STScnGpuFramebufferProps));
            if(f->props.ptr == NULL){
                printf("ERROR, ScnRender_jobFramebuffPush::ScnBuffer_malloc(STScnGpuFramebufferProps) failed.\n");
            } else if(!ScnRenderFbuffState_addTransform(f, &t)){
                printf("ERROR, ScnRender_jobFramebuffPush::ScnRenderFbuffState_addTransform failed.\n");
            } else {
                //ToDo: set props
                //ToDo: add command to set buffers and offsets
                opq->job.stackUse++;
                r = ScnTRUE;
            }
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRender_jobFramebuffPop(STScnRenderRef ref){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!opq->job.isActive){
        printf("ERROR, ScnRender_jobFramebuffPush::not active.\n");
    } else if(opq->job.stackUse > 0){
        printf("ERROR, ScnRender_jobFramebuffPush::nothing to pop.\n");
    } else {
        opq->job.stackUse--;
        //ToDo: set props
        //ToDo: add command to set buffers and offsets
        r = ScnTRUE;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//job transforms

ScnBOOL ScnRender_jobTransformPush(STScnRenderRef ref, STScnModelProps* tScene){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!opq->job.isActive){
        printf("ERROR, ScnRender_jobTransformPush::not active.\n");
    } else if(opq->job.stackUse > 0){
        printf("ERROR, ScnRender_jobTransformPush::no active framebuffer.\n");
    } else {
        STScnRenderFbuffState* f = &opq->job.stack.arr[opq->job.stackUse - 1];
        if(f->stackUse < 1){
            printf("ERROR, ScnRender_jobTransformPush::missing parent transform.\n");
        } else {
            const STScnGpuTransform t = ScnModelProps_toGpuTransform(tScene);
            const STScnGpuTransform tPrnt = f->stack.arr[f->stackUse - 1];
            const STScnGpuTransform tN = ScnGpuTransform_multiply(&tPrnt, &t);
            if(!ScnRenderFbuffState_addTransform(f, &tN)){
                printf("ERROR, ScnRender_jobTransformPush::ScnRenderFbuffState_addTransform failed.\n");
            } else {
                //ToDo: add render props to buffer
                r = ScnTRUE;
            }
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRender_jobTransformPop(STScnRenderRef ref){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!opq->job.isActive){
        printf("ERROR, ScnRender_jobTransformPop::not active.\n");
    } else if(opq->job.stackUse > 0){
        printf("ERROR, ScnRender_jobTransformPop::no active framebuffer.\n");
    } else {
        STScnRenderFbuffState* f = &opq->job.stack.arr[opq->job.stackUse - 1];
        if(f->stackUse < 2){
            printf("ERROR, ScnRender_jobTransformPop::nothing to pop.\n");
        } else {
            f->stackUse--;
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}


//job cmds

void ScnRender_cmdMaskModePush(STScnRenderRef ref){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnRenderCmd cmd;
        cmd.cmdId = ENScnRenderCmd_MaskModePush;
        ScnArray_addValue(opq->ctx, &opq->job.cmds, cmd, STScnRenderCmd);
    }
    ScnMutex_unlock(opq->mutex);
}

void ScnRender_cmdMaskModePop(STScnRenderRef ref){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnRenderCmd cmd;
        cmd.cmdId = ENScnRenderCmd_MaskModePop;
        ScnArray_addValue(opq->ctx, &opq->job.cmds, cmd, STScnRenderCmd);
    }
    ScnMutex_unlock(opq->mutex);
}

void ScnRender_cmdSetTexture(STScnRenderRef ref, const ScnUI32 index, const ScnUI32 tex /*const STScnGpuTextureRef tex*/){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnRenderCmd cmd;
        cmd.cmdId = ENScnRenderCmd_SetTexture;
        cmd.setTexture.index = index;
        cmd.setTexture.tex = tex;
        ScnArray_addValue(opq->ctx, &opq->job.cmds, cmd, STScnRenderCmd);
    }
    ScnMutex_unlock(opq->mutex);
}

void ScnRender_cmdSetVertsType(STScnRenderRef ref, const ENScnVertexType type){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnRenderCmd cmd;
        cmd.cmdId = ENScnRenderCmd_SetVertsType;
        cmd.setVertsType.type = type;
        ScnArray_addValue(opq->ctx, &opq->job.cmds, cmd, STScnRenderCmd);
    }
    ScnMutex_unlock(opq->mutex);
}

void ScnRender_cmdDawVerts(STScnRenderRef ref, const ENScnRenderShape mode, const ScnUI32 iFirst, const ScnUI32 count){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnRenderCmd cmd;
        cmd.cmdId = ENScnRenderCmd_DrawVerts;
        cmd.drawVerts.mode      = mode;
        cmd.drawVerts.iFirst    = iFirst;
        cmd.drawVerts.count     = count;
        ScnArray_addValue(opq->ctx, &opq->job.cmds, cmd, STScnRenderCmd);
    }
    ScnMutex_unlock(opq->mutex);
}

void ScnRender_cmdDawIndexes(STScnRenderRef ref, const ENScnRenderShape mode, const ScnUI32 iFirst, const ScnUI32 count){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnRenderCmd cmd;
        cmd.cmdId = ENScnRenderCmd_DrawIndexes;
        cmd.drawVerts.mode      = mode;
        cmd.drawVerts.iFirst    = iFirst;
        cmd.drawVerts.count     = count;
        ScnArray_addValue(opq->ctx, &opq->job.cmds, cmd, STScnRenderCmd);
    }
    ScnMutex_unlock(opq->mutex);
}

//gpu-render

ScnBOOL ScnRender_prepareNextRenderSlot(STScnRenderRef ref){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnGpuDevice_isNull(opq->gpuDev)){
        r = ScnTRUE;
        //
        ScnBOOL hasPtrs = ScnFALSE;
        if(r && !ScnBuffer_isNull(opq->viewPropsBuff) && !ScnBuffer_prepareNextRenderSlot(opq->viewPropsBuff, &hasPtrs)){
            printf("ERROR, ScnRender_prepareNextRenderSlot::ScnBuffer_prepareNextRenderSlot(viewPropsBuff) failed.\n");
            r = ScnFALSE;
        } else {
            //ToDO: eval 'hasPtrs' value
        }
        if(r && !ScnBuffer_isNull(opq->nodesPropsBuff) && !ScnBuffer_prepareNextRenderSlot(opq->nodesPropsBuff, &hasPtrs)){
            printf("ERROR, ScnRender_prepareNextRenderSlot::ScnBuffer_prepareNextRenderSlot(nodesPropsBuff) failed.\n");
            r = ScnFALSE;
        } else {
            //ToDO: eval 'hasPtrs' value
        }
        //vbuffs
        if(r && !ScnVertexbuffs_isNull(opq->vbuffs) && !ScnVertexbuffs_prepareNextRenderSlot(opq->vbuffs)){
            printf("ERROR, ScnRender_prepareNextRenderSlot::ScnVertexbuffs_prepareNextRenderSlot(vbuffs) failed.\n");
            r = ScnFALSE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//STScnRenderFbuffState

void ScnRenderFbuffState_init(STScnContextRef ctx, STScnRenderFbuffState* obj){
    memset(obj, 0, sizeof(*obj));
    ScnContext_set(&obj->ctx, ctx);
    //stack of transformations
    ScnArray_init(obj->ctx, &obj->stack, 256, 256, STScnGpuTransform);
}

void ScnRenderFbuffState_destroy(STScnRenderFbuffState* obj){
    //stack of transformations
    {
        ScnArray_destroy(obj->ctx, &obj->stack);
    }
    //
    if(!ScnContext_isNull(obj->ctx)){
        ScnContext_release(&obj->ctx);
        ScnContext_null(&obj->ctx);
    }
}

ScnBOOL ScnRenderFbuffState_addTransform(STScnRenderFbuffState* obj, const STScnGpuTransform* const t){
    if(obj->stackUse < obj->stack.use){
        obj->stack.arr[obj->stackUse++] = *t;
        return ScnTRUE;
    }
    if(NULL != ScnArray_addPtr(obj->ctx, &obj->stack, t, STScnGpuTransform)){
        obj->stackUse++;
        return ScnTRUE;
    }
    return ScnFALSE;
}

