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
    ScnContextRef           ctx;
    STScnAbsPtr             props;
    ScnArrayStruct(stack, STScnGpuTransform);   //stack of transformations
} STScnRenderFbuffState;

void ScnRenderFbuffState_init(ScnContextRef ctx, STScnRenderFbuffState* obj);
void ScnRenderFbuffState_destroy(STScnRenderFbuffState* obj);
//
SC_INLN void ScnRenderFbuffState_reset(STScnRenderFbuffState* obj){
    ScnArray_empty(&obj->stack);
}
SC_INLN ScnBOOL ScnRenderFbuffState_addTransform(STScnRenderFbuffState* obj, const STScnGpuTransform* const t){
    return ScnArray_addPtr(obj->ctx, &obj->stack, t, STScnGpuTransform) != NULL ? ScnTRUE : ScnFALSE;
}

//ENScnRenderJobObjType

typedef enum ENScnRenderJobObjType_ {
    ENScnRenderJobObjType_Unknown = 0,
    ENScnRenderJobObjType_Framebuff,
    ENScnRenderJobObjType_Vertexbuff,
    //
    ENScnRenderJobObjType_Count
} ENScnRenderJobObjType;

//STScnRenderJobObj

typedef struct STScnRenderJobObj_ {
    ENScnRenderJobObjType   type;
    union {
        ScnObjRef           objRef;     //generic ref (compatible will all bellow it)
        ScnFramebuffRef     framebuff;
        ScnVertexbuffRef    vertexbuff;
    };
} STScnRenderJobObj;

void ScnRenderJobObj_init(STScnRenderJobObj* obj);
void ScnRenderJobObj_destroy(STScnRenderJobObj* obj);
//
ScnBOOL ScnCompare_ScnRenderJobObj(const ENScnCompareMode mode, const void* data1, const void* data2, const ScnUI32 dataSz);

//STScnRenderOpq

typedef struct STScnRenderOpq_ {
    ScnContextRef       ctx;
    ScnMutexRef         mutex;
    ScnUI32             ammRenderSlots;
    //api
    struct {
        STScnApiItf     itf;
        void*           itfParam;
    } api;
    ScnGpuDeviceRef     gpuDev;
    ScnBufferRef        viewPropsBuff;  //buffer with viewport properties
    ScnBufferRef        nodesPropsBuff; //buffer with models
    ScnVertexbuffsRef   vbuffs;         //buffers with vertices and indices
    //job
    struct {
        ScnBOOL         isActive;       //jobStart() was called without a jobEnd()
        ScnUI32         stackUse;       //effective use of the stack-array (the array is reused between renders)
        ScnArrayStruct(stack, STScnRenderFbuffState);   //stack of framebuffers
        ScnArrayStruct(cmds, STScnRenderCmd);           //cmds sequence
        ScnArraySortedStruct(objs, STScnRenderJobObj);  //unique references of objects using for thias job
        //active (last state, helps to reduce redundant cmds)
        struct {
            ScnVertexbuffRef vbuff;
        } active;
    } job;
} STScnRenderOpq;

void ScnRender_jobResetActiveStateLockedOpq_(STScnRenderOpq* opq);

SC_INLN ScnBOOL ScnRender_addCmdLockedOpq_(STScnRenderOpq* opq, const STScnRenderCmd* const cmd){
    return ScnArray_addPtr(opq->ctx, &opq->job.cmds, cmd, STScnRenderCmd) != NULL ? ScnTRUE : ScnFALSE;
}
//

ScnSI32 ScnRender_getOpqSz(void){
    return (ScnSI32)sizeof(STScnRenderOpq);
}

void ScnRender_initZeroedOpq(ScnContextRef ctx, void* obj) {
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
        ScnArraySorted_init(opq->ctx, &opq->job.objs, 64, 128, STScnRenderJobObj, ScnCompare_ScnRenderJobObj);
    }
}

void ScnRender_destroyOpq(void* obj){
    STScnRenderOpq* opq = (STScnRenderOpq*)obj;
    //job
    {
        //objs
        {
            ScnArraySorted_foreach(&opq->job.objs, STScnRenderJobObj, o,
                ScnRenderJobObj_destroy(o);
            );
            opq->job.objs.use = 0;
        }
        //stack
        {
            ScnArray_foreach(&opq->job.stack, STScnRenderFbuffState, s,
                ScnRenderFbuffState_destroy(s);
            );
            opq->job.stack.use = 0;
            opq->job.stackUse = 0;
        }
        ScnArray_destroy(opq->ctx, &opq->job.stack);
        ScnArray_destroy(opq->ctx, &opq->job.cmds);
        ScnArraySorted_destroy(opq->ctx, &opq->job.objs);
    }
    ScnBuffer_releaseAndNullify(&opq->viewPropsBuff);
    ScnBuffer_releaseAndNullify(&opq->nodesPropsBuff);
    ScnVertexbuffs_releaseAndNullify(&opq->vbuffs);
    ScnGpuDevice_releaseAndNullify(&opq->gpuDev);
    //api
    {
        ScnMemory_setZeroSt(opq->api.itf, STScnApiItf);
        opq->api.itfParam = NULL;
    }
    //
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNullify(&opq->ctx);
}

//prepare

ScnBufferRef      ScnRender_allocDynamicBuffLockedOpq_(STScnRenderOpq* opq, const ScnUI32 itmSz, const ScnUI32 itmsPerBlock, const ScnUI32 ammRenderSlots);
ScnVertexbuffsRef ScnRender_allocVertexbuffsLockedOpq_(STScnRenderOpq* opq, const ScnUI32 ammRenderSlots);

ScnBOOL ScnRender_prepare(ScnRenderRef ref, const STScnApiItf* itf, void* itfParam){
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

ScnBOOL ScnRender_openDevice(ScnRenderRef ref, const STScnGpuDeviceCfg* cfg, const ScnUI32 ammRenderSlots){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(ScnGpuDevice_isNull(opq->gpuDev) && opq->api.itf.allocDevice != NULL && opq->ammRenderSlots == 0 && ammRenderSlots > 0){
        ScnGpuDeviceRef gpuDev = (*opq->api.itf.allocDevice)(opq->ctx, cfg);
        if(!ScnGpuDevice_isNull(gpuDev)){
            ScnGpuDevice_set(&opq->gpuDev, gpuDev);
            //
            opq->ammRenderSlots = ammRenderSlots;
            //initial buffers
            {
                ScnBufferRef viewPropsBuff = ScnObjRef_Zero;
                ScnBufferRef nodesPropsBuff = ScnObjRef_Zero;
                ScnVertexbuffsRef vbuffs = ScnObjRef_Zero;
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
                ScnBuffer_releaseAndNullify(&viewPropsBuff);
                ScnBuffer_releaseAndNullify(&nodesPropsBuff);
                ScnVertexbuffs_releaseAndNullify(&vbuffs);
            }
            //
            ScnGpuDevice_releaseAndNullify(&gpuDev);
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRender_hasOpenDevice(ScnRenderRef ref){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return !ScnGpuDevice_isNull(opq->gpuDev);
}

ScnModelRef ScnRender_allocModel(ScnRenderRef ref){
    ScnModelRef r = ScnObjRef_Zero;
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

//framebuffer

ScnFramebuffRef ScnRender_allocFramebuff(ScnRenderRef ref){
    ScnFramebuffRef r = ScnObjRef_Zero;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnGpuDevice_isNull(opq->gpuDev)){
        ScnFramebuffRef fb = ScnFramebuff_alloc(opq->ctx);
        if(ScnFramebuff_isNull(fb)){
            printf("ERROR, ScnRender_allocFramebuff::ScnFramebuff_alloc failed.\n");
        } else if(!ScnFramebuff_prepare(fb, opq->gpuDev)){
            printf("ERROR, ScnRender_allocFramebuff::ScnModel_setVertexBuffs failed.\n");
        } else {
            ScnFramebuff_set(&r, fb);
        }
        ScnFramebuff_releaseAndNullify(&fb);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//Vertices

ScnVertexbuffsRef ScnRender_getDefaultVertexbuffs(ScnRenderRef ref){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->vbuffs;
}

ScnVertexbuffsRef ScnRender_allocVertexbuffs(ScnRenderRef ref){
    ScnVertexbuffsRef r = ScnObjRef_Zero;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->ammRenderSlots > 0){
        r = ScnRender_allocVertexbuffsLockedOpq_(opq, opq->ammRenderSlots);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBufferRef ScnRender_allocDynamicBuffLockedOpq_(STScnRenderOpq* opq, const ScnUI32 itmSz, const ScnUI32 itmsPerBlock, const ScnUI32 ammRenderSlots){
    ScnBufferRef r        = ScnObjRef_Zero;
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

ScnVertexbuffsRef ScnRender_allocVertexbuffsLockedOpq_(STScnRenderOpq* opq, const ScnUI32 ammRenderSlots){
    ScnVertexbuffsRef rr = ScnObjRef_Zero;
    ScnBOOL r = ScnTRUE;
    ScnBufferRef vBuffs[ENScnVertexType_Count];
    ScnBufferRef iBuffs[ENScnVertexType_Count];
    ScnVertexbuffRef vbs[ENScnVertexType_Count];
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
            ScnBufferRef vertexBuff = vBuffs[i];
            ScnBufferRef idxsBuff = iBuffs[i];
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
            ScnVertexbuffsRef vbObj = ScnVertexbuffs_alloc(opq->ctx);
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
            ScnVertexbuffRef* b = vbs;
            const ScnVertexbuffRef* bAfterEnd = b + (sizeof(vbs) / sizeof(vbs[0]));
            while(b < bAfterEnd){
                ScnVertexbuff_release(b);
                ++b;
            }
        }
        //vBuffs
        {
            ScnBufferRef* b = vBuffs;
            const ScnBufferRef* bAfterEnd = b + (sizeof(vBuffs) / sizeof(vBuffs[0]));
            while(b < bAfterEnd){
                ScnBuffer_release(b);
                ++b;
            }
        }
        //iBuffs
        {
            ScnBufferRef* b = iBuffs;
            const ScnBufferRef* bAfterEnd = b + (sizeof(iBuffs) / sizeof(iBuffs[0]));
            while(b < bAfterEnd){
                ScnBuffer_release(b);
                ++b;
            }
        }
    }
    return rr;
}

//Buffers

ScnBufferRef ScnRender_allocBuffer(ScnRenderRef ref, const STScnGpuBufferCfg* cfg){ //allocates a new buffer
    ScnBufferRef r = ScnObjRef_Zero;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(cfg != NULL && opq->ammRenderSlots > 0){
        ScnBufferRef b = ScnBuffer_alloc(opq->ctx);
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
    
ScnBOOL ScnRender_jobAddUsedObjLockedOpq_(STScnRenderOpq* opq, const ENScnRenderJobObjType type, ScnObjRef* objRef);

void ScnRender_jobResetActiveStateLockedOpq_(STScnRenderOpq* opq){
    ScnVertexbuff_null(&opq->job.active.vbuff);
}

ScnBOOL ScnRender_jobStart(ScnRenderRef ref){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->job.isActive){
        printf("ERROR, ScnRender_jobStart:: already active.\n");
    } else {
        r = ScnTRUE;
        //empty objs
        {
            ScnArraySorted_foreach(&opq->job.objs, STScnRenderJobObj, o,
                ScnRenderJobObj_destroy(o);
            );
            opq->job.objs.use = 0;
        }
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
            //reset state
            ScnRender_jobResetActiveStateLockedOpq_(opq);
            //activate
            opq->job.isActive = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}
    
ScnBOOL ScnRender_jobEnd(ScnRenderRef ref){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!opq->job.isActive){
        printf("ERROR, ScnRender_jobEnd:: not active.\n");
    } else {
        //sync used objs:
        // - sync gpu-data from invalidated cpu-data.
        // - move to their next render slot.
        printf("ScnRender_jobEnd::%u objs, %u cmds.\n", opq->job.objs.use, opq->job.cmds.use);
        {
            ScnArraySorted_foreach(&opq->job.objs, STScnRenderJobObj, o,
                //ToDo: implement
                ScnRenderJobObj_destroy(o);
            );
            opq->job.objs.use = 0;
        }
        opq->job.isActive = ScnFALSE;
        r = ScnTRUE;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//job framebuffers

ScnBOOL ScnRender_jobFramebuffPush(ScnRenderRef ref, ScnFramebuffRef fbuff){
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
            ScnRenderFbuffState_reset(f);
            f->props    = ScnBuffer_malloc(opq->viewPropsBuff, sizeof(STScnGpuFramebufferProps));
            if(f->props.ptr == NULL){
                printf("ERROR, ScnRender_jobFramebuffPush::ScnBuffer_malloc(STScnGpuFramebufferProps) failed.\n");
            } else if(!ScnRenderFbuffState_addTransform(f, &t)){
                printf("ERROR, ScnRender_jobFramebuffPush::ScnRenderFbuffState_addTransform failed.\n");
            } else if(!ScnRender_jobAddUsedObjLockedOpq_(opq, ENScnRenderJobObjType_Framebuff, (ScnObjRef*)&fbuff)){
                printf("ERROR, ScnRender_jobFramebuffPush::ScnRender_jobAddUsedObjLockedOpq_ failed.\n");
            } else {
                STScnGpuFramebufferProps* props = (STScnGpuFramebufferProps*)f->props.ptr;
                memset(props, 0, sizeof(*props));
                props->size = ScnFramebuff_getSize(fbuff, &props->viewport);
                //ToDo: add command to set buffers and offsets
                opq->job.stackUse++;
                r = ScnTRUE;
            }
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRender_jobFramebuffPop(ScnRenderRef ref){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!opq->job.isActive){
        printf("ERROR, ScnRender_jobFramebuffPop::not active.\n");
    } else if(opq->job.stackUse <= 0){
        printf("ERROR, ScnRender_jobFramebuffPop::nothing to pop.\n");
    } else {
        opq->job.stackUse--;
        //ToDo: set props
        //ToDo: add command to set buffers and offsets
        r = ScnTRUE;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}


//job models

SC_INLN ScnBOOL ScnRender_job_addSetVertexBuffIfNecesaryLockedOpq_(STScnRenderOpq* opq, STScnRenderFbuffState* f, ScnVertexbuffRef vbuff){
    if(ScnVertexbuff_isSame(opq->job.active.vbuff, vbuff)){
        //redundant command
        return ScnTRUE;
    }
    //new command
    {
        STScnRenderCmd cmd;
        cmd.cmdId = ENScnRenderCmd_SetVertexBuff;
        cmd.setVertexBuff.ref = vbuff;
        if(!ScnRender_addCmdLockedOpq_(opq, &cmd)){
            printf("ERROR, ScnRender_job_addSetVertexBuffIfNecesaryLockedOpq_::ScnRender_addCmdLockedOpq_ failed.\n");
            return ScnFALSE;
        } else if(!ScnRender_jobAddUsedObjLockedOpq_(opq, ENScnRenderJobObjType_Vertexbuff, (ScnObjRef*)&vbuff)){
            printf("ERROR, ScnRender_job_addSetVertexBuffIfNecesaryLockedOpq_::ScnRender_jobAddUsedObjLockedOpq_ failed.\n");
        }
    }
    return ScnTRUE;
}

#define ScnRender_jobAddCommandDrawVertsLockeOpq_(OPQ, RENDERBUFF_STATE, DRAW_CMD, VERTEX_TYPE, VERTEX_TYPE_IDX) \
    if(ScnVertexbuffs_isNull((DRAW_CMD)->vbuffs)){ \
        printf("ERROR, ScnRender_jobModelAdd_addCommandsWithPropsLockedOpq_::cmd without ScnVertexbuffs.\n"); \
        r = ScnFALSE; \
    } else { \
        if(!ScnRender_job_addSetVertexBuffIfNecesaryLockedOpq_(OPQ, RENDERBUFF_STATE, ScnVertexbuffs_getVertexBuff((DRAW_CMD)->vbuffs, VERTEX_TYPE))){ \
            printf("ERROR, ScnRender_job_addSetVertexBuffIfNecesaryLockedOpq_ failed.\n"); \
            r = ScnFALSE; \
        } else { \
            STScnRenderCmd cmd; \
            cmd.cmdId               = ENScnRenderCmd_DrawVerts; \
            cmd.drawVerts.shape     = (DRAW_CMD)->shape; \
            cmd.drawVerts.iFirst    = (DRAW_CMD)->verts.v ## VERTEX_TYPE_IDX.idx; \
            cmd.drawVerts.count     = (DRAW_CMD)->verts.count; \
            if(!ScnRender_addCmdLockedOpq_(OPQ, &cmd)){ \
                printf("ERROR, ScnRender_job_addSetVertexBuffIfNecesaryLockedOpq_::ScnRender_addCmdLockedOpq_ failed.\n"); \
                r = ScnFALSE; \
            } \
        } \
    }

#define ScnRender_jobAddCommandDrawIndexesLockeOpq_(OPQ, RENDERBUFF_STATE, DRAW_CMD, VERTEX_TYPE, VERTEX_TYPE_IDX) \
    if(ScnVertexbuffs_isNull((DRAW_CMD)->vbuffs)){ \
        printf("ERROR, ScnRender_jobModelAdd_addCommandsWithPropsLockedOpq_::cmd without ScnVertexbuffs.\n"); \
        r = ScnFALSE; \
    } else { \
        if(!ScnRender_job_addSetVertexBuffIfNecesaryLockedOpq_(OPQ, RENDERBUFF_STATE, ScnVertexbuffs_getVertexBuff((DRAW_CMD)->vbuffs, VERTEX_TYPE))){ \
            printf("ERROR, ScnRender_job_addSetVertexBuffIfNecesaryLockedOpq_ failed.\n"); \
            r = ScnFALSE; \
        } else { \
            STScnRenderCmd cmd; \
            cmd.cmdId               = ENScnRenderCmd_DrawIndexes; \
            cmd.drawIndexes.shape   = (DRAW_CMD)->shape; \
            cmd.drawIndexes.iFirst  = (DRAW_CMD)->idxs.i ## VERTEX_TYPE_IDX.idx; \
            cmd.drawIndexes.count   = (DRAW_CMD)->idxs.count; \
            if(!ScnRender_addCmdLockedOpq_(OPQ, &cmd)){ \
                printf("ERROR, ScnRender_job_addSetVertexBuffIfNecesaryLockedOpq_::ScnRender_addCmdLockedOpq_ failed.\n"); \
                r = ScnFALSE; \
            } \
        } \
    }

ScnBOOL ScnRender_jobModelAdd_addCommandsWithTransformLockedOpq_(STScnRenderOpq* opq, STScnRenderFbuffState* f, const STScnGpuTransform* const transform, const STScnModelDrawCmd* const cmds, const ScnUI32 cmdsSz){
    ScnBOOL r = ScnFALSE;
    //add commands with this matrix (do not push it into the stack)
    STScnAbsPtr tNPtr = ScnBuffer_malloc(opq->nodesPropsBuff, sizeof(STScnGpuTransform));
    if(tNPtr.ptr == NULL){
        printf("ERROR, ScnRender_jobModelAdd_addCommandsWithTransformLockedOpq_::ScnBuffer_malloc failed.\n");
    } else {
        *((STScnGpuTransform*)tNPtr.ptr) = *transform;
        r = ScnTRUE;
        //add transform offset cmd
        if(r){
             STScnRenderCmd cmd;
             cmd.cmdId = ENScnRenderCmd_SetTransformOffset;
             cmd.setTransformOffset.offset = tNPtr.idx;
             if(!ScnRender_addCmdLockedOpq_(opq, &cmd)){
                 printf("ERROR, ScnRender_jobModelAdd_addCommandsWithTransformLockedOpq_::ScnRender_addCmdLockedOpq_ failed.\n");
                 r = ScnFALSE;
             }
         }
         //add model commands
        {
            const STScnModelDrawCmd* c = cmds;
            const STScnModelDrawCmd* cAfterEnd = c + cmdsSz;
            while(r && c < cAfterEnd){
                switch (c->type) {
                    case ENScnModelDrawCmdType_Undef:
                        //nop
                        break;
                    case ENScnModelDrawCmdType_v0: ScnRender_jobAddCommandDrawVertsLockeOpq_(opq, f, c, ENScnVertexType_Color, 0); break;
                    case ENScnModelDrawCmdType_v1: ScnRender_jobAddCommandDrawVertsLockeOpq_(opq, f, c, ENScnVertexType_Tex, 1); break;
                    case ENScnModelDrawCmdType_v2: ScnRender_jobAddCommandDrawVertsLockeOpq_(opq, f, c, ENScnVertexType_Tex2, 2); break;
                    case ENScnModelDrawCmdType_v3: ScnRender_jobAddCommandDrawVertsLockeOpq_(opq, f, c, ENScnVertexType_Tex3, 3); break;
                        //
                    case ENScnModelDrawCmdType_i0: ScnRender_jobAddCommandDrawIndexesLockeOpq_(opq, f, c, ENScnVertexType_Color, 0); break;
                    case ENScnModelDrawCmdType_i1: ScnRender_jobAddCommandDrawIndexesLockeOpq_(opq, f, c, ENScnVertexType_Tex, 1); break;
                    case ENScnModelDrawCmdType_i2: ScnRender_jobAddCommandDrawIndexesLockeOpq_(opq, f, c, ENScnVertexType_Tex2, 2);break;
                    case ENScnModelDrawCmdType_i3: ScnRender_jobAddCommandDrawIndexesLockeOpq_(opq, f, c, ENScnVertexType_Tex3, 3); break;
                    default:
                        SCN_ASSERT(ScnFALSE) //unimplemented cmd
                        break;
                }
                //ToDo: cmds to activate textures
                ++c;
            }
        }
    }
    return r;
}
    
ScnBOOL ScnRender_jobModelAdd_addCommandsWithPropsLockedOpq_(void* data, const STScnModelProps* const props, const STScnModelDrawCmd* const cmds, const ScnUI32 cmdsSz){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)data;
    if(opq != NULL){
        if(!opq->job.isActive){
            printf("ERROR, ScnRender_jobModelAdd_addCommandsWithPropsLockedOpq_::not active.\n");
        } else if(opq->job.stackUse <= 0){
            printf("ERROR, ScnRender_jobModelAdd_addCommandsWithPropsLockedOpq_::no active framebuffer.\n");
        } else {
            STScnRenderFbuffState* f = &opq->job.stack.arr[opq->job.stackUse - 1];
            if(f->stack.use < 1){
                printf("ERROR, ScnRender_jobModelAdd_addCommandsWithPropsLockedOpq_::missing parent transform.\n");
            } else {
                SCN_ASSERT(!ScnBuffer_isNull(opq->viewPropsBuff)) //program logic error
                SCN_ASSERT(!ScnBuffer_isNull(opq->nodesPropsBuff)) //program logic error
                const STScnGpuTransform t = ScnModelProps_toGpuTransform(props);
                const STScnGpuTransform tPrnt = f->stack.arr[f->stack.use - 1];
                const STScnGpuTransform tN = ScnGpuTransform_multiply(&tPrnt, &t);
                if(!ScnRender_jobModelAdd_addCommandsWithTransformLockedOpq_(opq, f, &tN, cmds, cmdsSz)){
                    printf("ERROR, ScnRender_jobModelAdd_addCommandsWithPropsLockedOpq_::ScnRender_jobModelAdd_addCommandsWithTransformLockedOpq_ failed.\n");
                } else {
                    r = ScnTRUE;
                }
            }
        }
    }
    return r;
}

ScnBOOL ScnRender_jobModelAdd(ScnRenderRef ref, ScnModelRef model){    //equivalent to push-and-pop
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!opq->job.isActive){
        printf("ERROR, ScnRender_jobFramebuffPop::not active.\n");
    } else if(opq->job.stackUse <= 0){
        printf("ERROR, ScnRender_jobFramebuffPop::no framebuffer is active.\n");
    } else {
        SCN_ASSERT(!ScnBuffer_isNull(opq->viewPropsBuff)) //program logic error
        SCN_ASSERT(!ScnBuffer_isNull(opq->nodesPropsBuff)) //program logic error
        STScnModelPushItf itf = STScnModelPushItf_Zero;
        itf.addCommandsWithProps = ScnRender_jobModelAdd_addCommandsWithPropsLockedOpq_;
        if(!ScnModel_sendRenderCmds(model, &itf, opq)){
            printf("ERROR, ScnRender_jobFramebuffPop::ScnModel_sendRenderCmds failed.\n");
        } else {
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRender_jobModelAdd_addCommandsWithPropsAndPushLockedOpq_(void* data, const STScnModelProps* const props, const STScnModelDrawCmd* const cmds, const ScnUI32 cmdsSz){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)data;
    if(opq != NULL){
        if(!opq->job.isActive){
            printf("ERROR, ScnRender_jobModelAdd_addCommandsWithPropsAndPushLockedOpq_::not active.\n");
        } else if(opq->job.stackUse > 0){
            printf("ERROR, ScnRender_jobModelAdd_addCommandsWithPropsAndPushLockedOpq_::no active framebuffer.\n");
        } else {
            STScnRenderFbuffState* f = &opq->job.stack.arr[opq->job.stackUse - 1];
            if(f->stack.use < 1){
                printf("ERROR, ScnRender_jobModelAdd_addCommandsWithPropsAndPushLockedOpq_::missing parent transform.\n");
            } else {
                SCN_ASSERT(!ScnBuffer_isNull(opq->viewPropsBuff)) //program logic error
                SCN_ASSERT(!ScnBuffer_isNull(opq->nodesPropsBuff)) //program logic error
                const STScnGpuTransform t = ScnModelProps_toGpuTransform(props);
                const STScnGpuTransform tPrnt = f->stack.arr[f->stack.use - 1];
                const STScnGpuTransform tN = ScnGpuTransform_multiply(&tPrnt, &t);
                if(!ScnRender_jobModelAdd_addCommandsWithTransformLockedOpq_(opq, f, &tN, cmds, cmdsSz)){
                    printf("ERROR, ScnRender_jobModelAdd_addCommandsWithPropsAndPushLockedOpq_::ScnRender_jobModelAdd_addCommandsWithTransformLockedOpq_ failed.\n");
                } else if(!ScnRenderFbuffState_addTransform(f, &tN)){
                    printf("ERROR, ScnRender_jobModelAdd_addCommandsWithPropsAndPushLockedOpq_::ScnRenderFbuffState_addTransform failed.\n");
                } else {
                    r = ScnTRUE;
                }
            }
        }
    }
    return r;
}

ScnBOOL ScnRender_jobModelPush(ScnRenderRef ref, ScnModelRef model){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!opq->job.isActive){
        printf("ERROR, ScnRender_jobModelPush::not active.\n");
    } else if(opq->job.stackUse <= 0){
        printf("ERROR, ScnRender_jobModelPush::no framebuffer is active.\n");
    } else {
        SCN_ASSERT(!ScnBuffer_isNull(opq->viewPropsBuff)) //program logic error
        SCN_ASSERT(!ScnBuffer_isNull(opq->nodesPropsBuff)) //program logic error
        STScnModelPushItf itf = STScnModelPushItf_Zero;
        itf.addCommandsWithProps = ScnRender_jobModelAdd_addCommandsWithPropsAndPushLockedOpq_;
        if(!ScnModel_sendRenderCmds(model, &itf, opq)){
            printf("ERROR, ScnRender_jobModelPush::ScnModel_sendRenderCmds failed.\n");
        } else {
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRender_jobModelPop(ScnRenderRef ref){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!opq->job.isActive){
        printf("ERROR, ScnRender_jobModelPop::not active.\n");
    } else if(opq->job.stackUse <= 0){
        printf("ERROR, ScnRender_jobModelPop::no framebuffer is active.\n");
    } else {
        SCN_ASSERT(!ScnBuffer_isNull(opq->viewPropsBuff)) //program logic error
        SCN_ASSERT(!ScnBuffer_isNull(opq->nodesPropsBuff)) //program logic error
        STScnRenderFbuffState* f = &opq->job.stack.arr[opq->job.stackUse - 1];
        if(f->stack.use < 2){
            printf("ERROR, ScnRender_jobModelPop::nothing to pop.\n");
        } else {
            f->stack.use--;
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//job transforms

ScnBOOL ScnRender_jobTransformPush(ScnRenderRef ref, STScnModelProps* tScene){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!opq->job.isActive){
        printf("ERROR, ScnRender_jobTransformPush::not active.\n");
    } else if(opq->job.stackUse > 0){
        printf("ERROR, ScnRender_jobTransformPush::no active framebuffer.\n");
    } else {
        STScnRenderFbuffState* f = &opq->job.stack.arr[opq->job.stackUse - 1];
        if(f->stack.use < 1){
            printf("ERROR, ScnRender_jobTransformPush::missing parent transform.\n");
        } else {
            const STScnGpuTransform t = ScnModelProps_toGpuTransform(tScene);
            const STScnGpuTransform tPrnt = f->stack.arr[f->stack.use - 1];
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

ScnBOOL ScnRender_jobTransformPop(ScnRenderRef ref){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!opq->job.isActive){
        printf("ERROR, ScnRender_jobTransformPop::not active.\n");
    } else if(opq->job.stackUse > 0){
        printf("ERROR, ScnRender_jobTransformPop::no active framebuffer.\n");
    } else {
        STScnRenderFbuffState* f = &opq->job.stack.arr[opq->job.stackUse - 1];
        if(f->stack.use < 2){
            printf("ERROR, ScnRender_jobTransformPop::nothing to pop.\n");
        } else {
            f->stack.use--;
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}


//job cmds

/*
void ScnRender_cmdMaskModePush(ScnRenderRef ref){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnRenderCmd cmd;
        cmd.cmdId = ENScnRenderCmd_MaskModePush;
        ScnArray_addValue(opq->ctx, &opq->job.cmds, cmd, STScnRenderCmd);
    }
    ScnMutex_unlock(opq->mutex);
}

void ScnRender_cmdMaskModePop(ScnRenderRef ref){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnRenderCmd cmd;
        cmd.cmdId = ENScnRenderCmd_MaskModePop;
        ScnArray_addValue(opq->ctx, &opq->job.cmds, cmd, STScnRenderCmd);
    }
    ScnMutex_unlock(opq->mutex);
}

void ScnRender_cmdSetTexture(ScnRenderRef ref, const ScnUI32 index, ScnTextureRef tex){
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

void ScnRender_cmdSetVertsType(ScnRenderRef ref, const ENScnVertexType type){
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

void ScnRender_cmdDawVerts(ScnRenderRef ref, const ENScnRenderShape mode, const ScnUI32 iFirst, const ScnUI32 count){
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

void ScnRender_cmdDawIndexes(ScnRenderRef ref, const ENScnRenderShape mode, const ScnUI32 iFirst, const ScnUI32 count){
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
*/

//gpu-render

/*
ScnBOOL ScnRender_prepareNextRenderSlot(ScnRenderRef ref){
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
*/

//STScnRenderFbuffState

void ScnRenderFbuffState_init(ScnContextRef ctx, STScnRenderFbuffState* obj){
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
    ScnContext_releaseAndNullify(&obj->ctx);
}

//STScnRenderJobObj

void ScnRenderJobObj_init(STScnRenderJobObj* obj){
    memset(obj, 0, sizeof(*obj));
}

void ScnRenderJobObj_destroy(STScnRenderJobObj* obj){
    switch(obj->type){
        case ENScnRenderJobObjType_Unknown:
            SCN_ASSERT(obj->objRef.ptr == NULL);
            break;
        case ENScnRenderJobObjType_Framebuff:
        case ENScnRenderJobObjType_Vertexbuff:
            ScnObjRef_releaseAndNullify(&obj->objRef);
            break;
        default:
            SCN_ASSERT(ScnFALSE); //unimplemented value
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

ScnBOOL ScnRender_jobAddUsedObjLockedOpq_(STScnRenderOpq* opq, const ENScnRenderJobObjType type, ScnObjRef* objRef){
    ScnBOOL r = ScnFALSE;
    STScnRenderJobObj usedObj;
    usedObj.type    = type;
    usedObj.objRef  = *objRef;
    const ScnSI32 iFnd = ScnArraySorted_indexOf(&opq->job.objs, &usedObj);
    if(iFnd < 0){
        //first time used, add to array
        ScnRenderJobObj_init(&usedObj);
        usedObj.type = type;
        ScnObjRef_set(&usedObj.objRef, *objRef);
        if(NULL == ScnArraySorted_addPtr(opq->ctx, &opq->job.objs, &usedObj, STScnRenderJobObj)){
            printf("ERROR, ScnRender_jobFramebuffPush::ScnArraySorted_addPtr(usedObj) failed.\n");
            ScnRenderJobObj_destroy(&usedObj);
        } else {
            r = ScnTRUE;
        }
    }
    return r;
}
