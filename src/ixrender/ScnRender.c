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
#include "ixrender/gpu/ScnGpuModelProps2d.h"

//STScnRenderFbuffProps

#define STScnRenderFbuffProps_Zero   { STScnGpuFramebuffProps_Zero, STScnAbsPtr_Zero }

typedef struct STScnRenderFbuffProps {
    STScnGpuFramebuffProps  props;
    STScnAbsPtr             ptr;
} STScnRenderFbuffProps;

//STScnRenderFbuffState

typedef struct STScnRenderFbuffState {
    ScnContextRef       ctx;
    ScnFramebuffRef     fbuff;
    //stacks
    struct {
        ScnArrayStruct(props, STScnRenderFbuffProps);       //stack of props (viewport, ortho, ...)
        ScnArrayStruct(transforms, STScnGpuModelProps2d);   //stack of transformations (color, matrix, ...)
    } stacks;
    //active (last state, helps to reduce redundant cmds)
    struct {
        ScnVertexbuffRef vbuff;
        //texs
        struct {
            ScnTextureRef refs[ENScnGpuTextureIdx_Count];
            ScnUI32     changesSeq;
        } texs;
    } active;
} STScnRenderFbuffState;

void ScnRenderFbuffState_init(ScnContextRef ctx, STScnRenderFbuffState* obj);
void ScnRenderFbuffState_destroy(STScnRenderFbuffState* obj);
//

SC_INLN void ScnRenderFbuffState_resetActiveState(STScnRenderFbuffState* obj){
    ScnMemory_setZeroSt(obj->active);
}


SC_INLN void ScnRenderFbuffState_reset(STScnRenderFbuffState* obj){
    ScnArray_empty(&obj->stacks.props);
    ScnArray_empty(&obj->stacks.transforms);
    ScnRenderFbuffState_resetActiveState(obj);
}

SC_INLN ScnBOOL ScnRenderFbuffState_addProps(STScnRenderFbuffState* obj, const STScnRenderFbuffProps* const p){
    return ScnArray_addPtr(obj->ctx, &obj->stacks.props, p, STScnRenderFbuffProps) != NULL ? ScnTRUE : ScnFALSE;
}

SC_INLN ScnBOOL ScnRenderFbuffState_addTransform(STScnRenderFbuffState* obj, const STScnGpuModelProps2d* const t){
    return ScnArray_addPtr(obj->ctx, &obj->stacks.transforms, t, STScnGpuModelProps2d) != NULL ? ScnTRUE : ScnFALSE;
}

//ENScnRenderJobObjType

typedef enum ENScnRenderJobObjType {
    ENScnRenderJobObjType_Unknown = 0,
    ENScnRenderJobObjType_Buff,
    ENScnRenderJobObjType_Framebuff,
    ENScnRenderJobObjType_Vertexbuff,
    ENScnRenderJobObjType_Texture,
    //
    ENScnRenderJobObjType_Count
} ENScnRenderJobObjType;

//STScnRenderJobObj

typedef struct STScnRenderJobObj {
    ENScnRenderJobObjType   type;
    union {
        ScnObjRef           objRef;     //generic ref (compatible will all bellow it)
        ScnBufferRef        buff;
        ScnFramebuffRef     framebuff;
        ScnVertexbuffRef    vertexbuff;
        ScnTextureRef       texture;
    };
} STScnRenderJobObj;

void ScnRenderJobObj_init(STScnRenderJobObj* obj);
void ScnRenderJobObj_destroy(STScnRenderJobObj* obj);
//
ScnBOOL ScnCompare_ScnRenderJobObj(const ENScnCompareMode mode, const void* data1, const void* data2, const ScnUI32 dataSz);

//STScnRenderOpq

typedef struct STScnRenderOpq {
    ScnContextRef       ctx;
    ScnMutexRef         mutex;
    ScnUI32             ammRenderSlots;
    //api
    struct {
        STScnApiItf     itf;
        void*           itfParam;
    } api;
    ScnGpuDeviceRef     gpuDev;
    STScnGpuDeviceDesc  gpuDevDesc;
    ScnBufferRef        fbPropsBuff;    //buffer with viewport properties
    ScnBufferRef        mdlsPropsBuff;  //buffer with models
    ScnVertexbuffsRef   vbuffs;         //buffers with vertices and indices
    //job
    struct {
        ScnBOOL         isActive;       //jobStart() was called without a jobEnd()
        ScnUI32         stackUse;       //effective use of the stack-array (the array is reused between renders)
        ScnArrayStruct(stack, STScnRenderFbuffState);   //stack of framebuffers
        ScnArrayStruct(cmds, STScnRenderCmd);           //cmds sequence
        ScnArraySortedStruct(objs, STScnRenderJobObj);  //unique references of objects using for thias job
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
    ScnBuffer_releaseAndNull(&opq->fbPropsBuff);
    ScnBuffer_releaseAndNull(&opq->mdlsPropsBuff);
    ScnVertexbuffs_releaseAndNull(&opq->vbuffs);
    ScnGpuDevice_releaseAndNull(&opq->gpuDev);
    //api
    {
        ScnMemory_setZeroSt(opq->api.itf);
        opq->api.itfParam = NULL;
    }
    //
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNull(&opq->ctx);
}

//prepare

ScnBufferRef      ScnRender_allocDynamicBuffLockedOpq_(STScnRenderOpq* opq, const ScnUI32 offsetsAlignment, const ScnUI32 itmSz, const ScnUI32 itmsPerBlock, const ScnUI32 ammRenderSlots);
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
            opq->gpuDevDesc = ScnGpuDevice_getDesc(opq->gpuDev);
            //
            opq->ammRenderSlots = ammRenderSlots;
            //initial buffers
            {
                ScnBufferRef fbPropsBuff = ScnObjRef_Zero;
                ScnBufferRef mdlsPropsBuff = ScnObjRef_Zero;
                ScnVertexbuffsRef vbuffs = ScnObjRef_Zero;
                //
                //Note: Metal requires buffers offset to be 32 bytes aligned
                fbPropsBuff = ScnRender_allocDynamicBuffLockedOpq_(opq, 32, sizeof(STScnGpuFramebuffProps), 32, ammRenderSlots);
                if(ScnBuffer_isNull(fbPropsBuff)){
                    printf("ERROR, ScnRender_allocDynamicBuffLockedOpq_(fbPropsBuff) failed.\n");
                } else {
                    mdlsPropsBuff = ScnRender_allocDynamicBuffLockedOpq_(opq, 32, sizeof(STScnGpuModelProps2d), 128, ammRenderSlots);
                    if(ScnBuffer_isNull(mdlsPropsBuff)){
                        printf("ERROR, ScnRender_allocDynamicBuffLockedOpq_(mdlsPropsBuff) failed.\n");
                    } else {
                        vbuffs = ScnRender_allocVertexbuffsLockedOpq_(opq, ammRenderSlots);
                        if(ScnVertexbuffs_isNull(vbuffs)){
                            printf("ERROR, ScnRender_allocVertexbuffsLockedOpq_(vbuffs) failed.\n");
                        } else {
                            ScnBuffer_set(&opq->fbPropsBuff, fbPropsBuff);
                            ScnBuffer_set(&opq->mdlsPropsBuff, mdlsPropsBuff);
                            ScnVertexbuffs_set(&opq->vbuffs, vbuffs);
                            r = ScnTRUE;
                        }
                    }
                }
                ScnBuffer_releaseAndNull(&fbPropsBuff);
                ScnBuffer_releaseAndNull(&mdlsPropsBuff);
                ScnVertexbuffs_releaseAndNull(&vbuffs);
            }
            //
            ScnGpuDevice_releaseAndNull(&gpuDev);
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRender_hasOpenDevice(ScnRenderRef ref){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return !ScnGpuDevice_isNull(opq->gpuDev);
}

void* ScnRender_getApiDevice(ScnRenderRef ref){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return !ScnGpuDevice_isNull(opq->gpuDev) ? ScnGpuDevice_getApiDevice(opq->gpuDev) : NULL;
}

STScnGpuDeviceDesc ScnRender_getDeviceDesc(ScnRenderRef ref){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return !ScnGpuDevice_isNull(opq->gpuDev) ? ScnGpuDevice_getDesc(opq->gpuDev) : (STScnGpuDeviceDesc)STScnGpuDeviceDesc_Zero;
}

ScnModel2dRef ScnRender_allocModel(ScnRenderRef ref){
    ScnModel2dRef r = ScnObjRef_Zero;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnVertexbuffs_isNull(opq->vbuffs)){
        r = ScnModel2d_alloc(opq->ctx);
        if(ScnModel2d_isNull(r)){
            printf("ERROR, ScnRender_allocModel::ScnModel2d_alloc failed.\n");
        } else if(!ScnModel2d_setVertexBuffs(r, opq->vbuffs)){
            printf("ERROR, ScnRender_allocModel::ScnModel2d_setVertexBuffs failed.\n");
            ScnModel2d_releaseAndNull(&r);
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
            printf("ERROR, ScnRender_allocFramebuff::ScnModel2d_setVertexBuffs failed.\n");
        } else {
            ScnFramebuff_set(&r, fb);
        }
        ScnFramebuff_releaseAndNull(&fb);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//texture

ScnTextureRef ScnRender_allocTexture(ScnRenderRef ref, const ENScnResourceMode mode, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const optSrcProps, const void* optSrcData){
    ScnTextureRef r = ScnObjRef_Zero;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnGpuDevice_isNull(opq->gpuDev) && opq->ammRenderSlots > 0){
        ScnTextureRef fb = ScnTexture_alloc(opq->ctx);
        if(ScnTexture_isNull(fb)){
            printf("ERROR, ScnRender_allocTexture::ScnTexture_alloc failed.\n");
        } else if(!ScnTexture_prepare(fb, opq->gpuDev, mode, (mode == ENScnResourceMode_Dynamic  ? opq->ammRenderSlots : 1), cfg, optSrcProps, optSrcData)){
            printf("ERROR, ScnRender_allocTexture::ScnTexture_prepare failed.\n");
        } else {
            ScnTexture_set(&r, fb);
        }
        ScnTexture_releaseAndNull(&fb);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//sampler

ScnGpuSamplerRef ScnRender_allocSampler(ScnRenderRef ref, const STScnGpuSamplerCfg* const cfg){
    ScnGpuSamplerRef r = ScnObjRef_Zero;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnGpuDevice_isNull(opq->gpuDev)){
        ScnGpuSamplerRef s = ScnGpuDevice_allocSampler(opq->gpuDev, cfg);
        if(ScnGpuSampler_isNull(s)){
            printf("ERROR, ScnRender_allocTexture::ScnGpuDevice_allocSampler failed.\n");
            ScnGpuSampler_set(&r, s);
        }
        ScnGpuSampler_releaseAndNull(&s);
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

ScnBufferRef ScnRender_allocDynamicBuffLockedOpq_(STScnRenderOpq* opq, const ScnUI32 offsetsAlignment, const ScnUI32 itmSz, const ScnUI32 itmsPerBlock, const ScnUI32 ammRenderSlots){
    ScnBufferRef r        = ScnObjRef_Zero;
    STScnGpuBufferCfg cfg   = STScnGpuBufferCfg_Zero;
    cfg.mem.idxsAlign       = (itmSz + offsetsAlignment - 1) / offsetsAlignment * offsetsAlignment;
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
        ScnBuffer_releaseAndNull(&r);
    }
    return r;
}

ScnVertexbuffsRef ScnRender_allocVertexbuffsLockedOpq_(STScnRenderOpq* opq, const ScnUI32 ammRenderSlots){
    ScnVertexbuffsRef rr = ScnObjRef_Zero;
    ScnBOOL r = ScnTRUE;
    ScnBufferRef vBuffs[ENScnVertexType_Count];
    ScnBufferRef iBuffs[ENScnVertexType_Count];
    ScnVertexbuffRef vbs[ENScnVertexType_Count];
    ScnMemory_setZeroSt(vBuffs);
    ScnMemory_setZeroSt(iBuffs);
    ScnMemory_setZeroSt(vbs);
    //initial bufffers
    if(r){
        ScnSI32 i; for(i = 0; i < ENScnVertexType_Count; i++){
            ScnUI32 itmSz = 0, ammPerBock = 0;
            switch(i){
                case ENScnVertexType_2DColor:
                    itmSz = sizeof(STScnVertex2D);
                    ammPerBock = 256;
                    break;
                case ENScnVertexType_2DTex:
                    itmSz = sizeof(STScnVertex2DTex);
                    ammPerBock = 1024;
                    break;
                case ENScnVertexType_2DTex2:
                    itmSz = sizeof(STScnVertex2DTex2);
                    ammPerBock = 256;
                    break;
                case ENScnVertexType_2DTex3:
                    itmSz = sizeof(STScnVertex2DTex3);
                    ammPerBock = 256;
                    break;
                default: SCN_ASSERT(ScnFALSE); r = ScnFALSE; break; //missing implementation
            }
            //vertex buffer
            if(r){
                vBuffs[i] = ScnRender_allocDynamicBuffLockedOpq_(opq, itmSz, itmSz, ammPerBock, ammRenderSlots);
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
                iBuffs[i] = ScnRender_allocDynamicBuffLockedOpq_(opq, itmSz, itmSz, ammPerBock, ammRenderSlots);
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
                case ENScnVertexType_2DColor: cfg.szPerRecord = sizeof(STScnVertex2D); break;
                case ENScnVertexType_2DTex: cfg.szPerRecord = sizeof(STScnVertex2DTex); break;
                case ENScnVertexType_2DTex2: cfg.szPerRecord = sizeof(STScnVertex2DTex2); break;
                case ENScnVertexType_2DTex3: cfg.szPerRecord = sizeof(STScnVertex2DTex3); break;
                default: SCN_ASSERT(ScnFALSE) r = ScnFALSE; break; //missing implementation
            }
            //SCN_ASSERT((cfg.szPerRecord % 4) == 0)
            //elems
            switch(i){
                case ENScnVertexType_2DTex3:
                    cfg.texCoords[2].amm    = 2;
                    cfg.texCoords[2].type   = ENScnGpuDataType_FLOAT32;
                    cfg.texCoords[2].offset = ScnVertex2DTex3_IDX_tex3_x;
                case ENScnVertexType_2DTex2:
                    cfg.texCoords[1].amm    = 2;
                    cfg.texCoords[1].type   = ENScnGpuDataType_FLOAT32;
                    cfg.texCoords[1].offset = ScnVertex2DTex2_IDX_tex2_x;
                case ENScnVertexType_2DTex:
                    cfg.texCoords[0].amm    = 2;
                    cfg.texCoords[0].type   = ENScnGpuDataType_FLOAT32;
                    cfg.texCoords[0].offset = ScnVertex2DTex_IDX_tex_x;
                default:
                    //color
                    cfg.color.amm           = 4;
                    cfg.color.type          = ENScnGpuDataType_UI8;
                    cfg.color.offset        = ScnVertex2D_IDX_color;
                    //coord
                    cfg.coord.amm           = 2;
                    cfg.coord.type          = ENScnGpuDataType_FLOAT32;
                    cfg.coord.offset        = ScnVertex2D_IDX_x;
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
            ScnVertexbuffs_releaseAndNull(&vbObj);
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

ScnBufferRef ScnRender_allocBuffer(ScnRenderRef ref, const STScnGpuBufferCfg* const cfg){ //allocates a new buffer
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
        ScnBuffer_releaseAndNull(&b);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//job
    
ScnBOOL ScnRender_jobAddUsedObjLockedOpq_(STScnRenderOpq* opq, const ENScnRenderJobObjType type, ScnObjRef* objRef);

void ScnRender_jobResetActiveStateLockedOpq_(STScnRenderOpq* opq){
    opq->job.stackUse = 0;
    ScnArray_empty(&opq->job.cmds);
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
        //empty fbPropsBuff
        if(r && (ScnBuffer_isNull(opq->fbPropsBuff) || !ScnBuffer_clear(opq->fbPropsBuff) || !ScnBuffer_invalidateAll(opq->fbPropsBuff))){
            printf("ERROR, ScnRender_jobStart::ScnBuffer_clear(fbPropsBuff) failed.\n");
            r = ScnFALSE;
        }
        //empty mdlsPropsBuff
        if(r && (ScnBuffer_isNull(opq->mdlsPropsBuff) || !ScnBuffer_clear(opq->mdlsPropsBuff) || !ScnBuffer_invalidateAll(opq->mdlsPropsBuff))){
            printf("ERROR, ScnRender_jobStart::ScnBuffer_clear(mdlsPropsBuff) failed.\n");
            r = ScnFALSE;
        }
        //empty arrays
        if(r){
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
        //printf("ScnRender_jobEnd::%u objs, %u cmds.\n", opq->job.objs.use, opq->job.cmds.use);
        r = ScnTRUE;
        //register root buffers as used objects
        if(r){
            SCN_ASSERT(!ScnBuffer_isNull(opq->fbPropsBuff)) //program logic error
            SCN_ASSERT(!ScnBuffer_isNull(opq->mdlsPropsBuff)) //program logic error
            if(!ScnRender_jobAddUsedObjLockedOpq_(opq, ENScnRenderJobObjType_Buff, (ScnObjRef*)&opq->fbPropsBuff)){
                printf("ERROR, ScnRender_jobEnd::ScnRender_jobAddUsedObjLockedOpq_ failed.\n");
                r = ScnFALSE;
            } else if(!ScnRender_jobAddUsedObjLockedOpq_(opq, ENScnRenderJobObjType_Buff, (ScnObjRef*)&opq->mdlsPropsBuff)){
                printf("ERROR, ScnRender_jobEnd::ScnRender_jobAddUsedObjLockedOpq_ failed.\n");
                r = ScnFALSE;
            }
        }
        //sync used objects' gpu-data
        if(r){
            ScnArraySorted_foreach(&opq->job.objs, STScnRenderJobObj, o,
                switch(o->type){
                    case ENScnRenderJobObjType_Unknown: break;
                    case ENScnRenderJobObjType_Buff:
                        if(!ScnBuffer_prepareCurrentRenderSlot(o->buff, NULL)){
                            printf("ScnRender_jobEnd::ScnBuffer_prepareCurrentRenderSlot failed.\n");
                            r = ScnFALSE;
                            break;
                        }
                        break;
                    case ENScnRenderJobObjType_Framebuff:
                        if(!ScnFramebuff_prepareCurrentRenderSlot(o->framebuff)){
                            printf("ScnRender_jobEnd::ScnFramebuff_prepareCurrentRenderSlot failed.\n");
                            r = ScnFALSE;
                            break;
                        }
                        break;
                    case ENScnRenderJobObjType_Vertexbuff:
                        if(!ScnVertexbuff_prepareCurrentRenderSlot(o->vertexbuff)){
                            printf("ScnRender_jobEnd::ScnVertexbuff_prepareCurrentRenderSlot failed.\n");
                            r = ScnFALSE;
                            break;
                        }
                        break;
                    case ENScnRenderJobObjType_Texture:
                        if(!ScnTexture_prepareCurrentRenderSlot(o->texture)){
                            printf("ScnRender_jobEnd::ScnTexture_prepareCurrentRenderSlot failed.\n");
                            r = ScnFALSE;
                            break;
                        }
                        break;
                    default:
                        SCN_ASSERT(ScnFALSE) //missing implementation
                        break;
                }
            );
        }
        //send render commands to gpu
        if(r){
            ScnGpuBufferRef fbPropsBuff = ScnBuffer_getCurrentRenderSlot(opq->fbPropsBuff);
            ScnGpuBufferRef mdlsPropsBuff = ScnBuffer_getCurrentRenderSlot(opq->mdlsPropsBuff);
            if(ScnGpuBuffer_isNull(fbPropsBuff) || ScnGpuBuffer_isNull(mdlsPropsBuff)){
                //Nothing to render
            } else if(!ScnGpuDevice_render(opq->gpuDev, fbPropsBuff, mdlsPropsBuff, opq->job.cmds.arr, opq->job.cmds.use)){
                printf("ScnRender_jobEnd::ScnGpuDevice_render failed.\n");
                r = ScnFALSE;
            }
        }
        //move to next render slot
        if(r){
            ScnArraySorted_foreach(&opq->job.objs, STScnRenderJobObj, o,
                switch(o->type){
                    case ENScnRenderJobObjType_Unknown: break;
                    case ENScnRenderJobObjType_Buff:
                        if(!ScnBuffer_moveToNextRenderSlot(o->buff)){
                            printf("ScnRender_jobEnd::ScnBuffer_moveToNextRenderSlot failed.\n");
                            r = ScnFALSE;
                            break;
                        }
                        break;
                    case ENScnRenderJobObjType_Framebuff:
                        if(!ScnFramebuff_moveToNextRenderSlot(o->framebuff)){
                            printf("ScnRender_jobEnd::ScnFramebuff_moveToNextRenderSlot failed.\n");
                            r = ScnFALSE;
                            break;
                        }
                        break;
                    case ENScnRenderJobObjType_Vertexbuff:
                        if(!ScnVertexbuff_moveToNextRenderSlot(o->vertexbuff)){
                            printf("ScnRender_jobEnd::ScnVertexbuff_moveToNextRenderSlot failed.\n");
                            r = ScnFALSE;
                            break;
                        }
                        break;
                    case ENScnRenderJobObjType_Texture:
                        if(!ScnTexture_moveToNextRenderSlot(o->texture)){
                            printf("ScnRender_jobEnd::ScnTexture_moveToNextRenderSlot failed.\n");
                            r = ScnFALSE;
                            break;
                        }
                        break;
                    default:
                        SCN_ASSERT(ScnFALSE) //missing implementation
                        break;
                }
            );
        }
        //cleanup
        {
            ScnArraySorted_foreach(&opq->job.objs, STScnRenderJobObj, o,
                //ToDo: implement
                ScnRenderJobObj_destroy(o);
            );
            opq->job.objs.use = 0;
        }
        opq->job.isActive = ScnFALSE;
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
    } else if(!ScnBuffer_isNull(opq->fbPropsBuff)){
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
            STScnGpuModelProps2d t = STScnGpuModelProps2d_Identity;
            STScnRenderFbuffProps props = STScnRenderFbuffProps_Zero;
            props.props = ScnFramebuff_getProps(fbuff);
            props.ptr   = ScnBuffer_malloc(opq->fbPropsBuff, sizeof(STScnGpuFramebuffProps));
            //
            ScnRenderFbuffState_reset(f);
            f->fbuff    = fbuff;
            //
            if(props.ptr.ptr == NULL){
                printf("ERROR, ScnRender_jobFramebuffPush::ScnBuffer_malloc(STScnGpuFramebuffProps) failed.\n");
            } else if(!ScnRenderFbuffState_addProps(f, &props)){
                printf("ERROR, ScnRender_jobFramebuffPush::ScnRenderFbuffState_addProps failed.\n");
            } else if(!ScnRenderFbuffState_addTransform(f, &t)){
                printf("ERROR, ScnRender_jobFramebuffPush::ScnRenderFbuffState_addTransform failed.\n");
            } else if(!ScnRender_jobAddUsedObjLockedOpq_(opq, ENScnRenderJobObjType_Framebuff, (ScnObjRef*)&fbuff)){
                printf("ERROR, ScnRender_jobFramebuffPush::ScnRender_jobAddUsedObjLockedOpq_ failed.\n");
            } else {
                //add cmd
                STScnRenderCmd cmd;
                cmd.cmdId                       = ENScnRenderCmd_ActivateFramebuff;
                cmd.activateFramebuff.ref       = fbuff;
                cmd.activateFramebuff.offset    = props.ptr.idx;
                cmd.activateFramebuff.viewport  = props.props.viewport;
                if(!ScnRender_addCmdLockedOpq_(opq, &cmd)){
                    printf("ERROR, ScnRender_jobFramebuffPush::ScnRender_addCmdLockedOpq_ failed.\n");
                } else {
                    //push fb props
                    {
                        STScnGpuFramebuffProps* buffPtr = (STScnGpuFramebuffProps*)props.ptr.ptr;
                        *buffPtr = props.props;
                        opq->job.stackUse++;
                    }
                    r = ScnTRUE;
                }
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
        r = ScnTRUE;
        opq->job.stackUse--;
        //reset fb active state (to force initial state)
        if(opq->job.stackUse > 0){
            STScnRenderFbuffState* f = &opq->job.stack.arr[opq->job.stackUse - 1];
            ScnRenderFbuffState_resetActiveState(f);
            //add cmd
            if(f->stacks.props.use <= 0){
                //program logic error (should be previously validated)
                printf("ERROR, ScnRender_jobFramebuffPop::prev-framebuff has not properties in stack.\n");
                r = ScnFALSE;
            } else {
                const STScnRenderFbuffProps* props = &f->stacks.props.arr[f->stacks.props.use - 1];
                STScnRenderCmd cmd;
                cmd.cmdId                       = ENScnRenderCmd_ActivateFramebuff;
                cmd.activateFramebuff.ref       = f->fbuff;
                cmd.activateFramebuff.offset    = props->ptr.idx;
                cmd.activateFramebuff.viewport  = props->props.viewport;
                if(!ScnRender_addCmdLockedOpq_(opq, &cmd)){
                    printf("ERROR, ScnRender_jobFramebuffPop::ScnRender_addCmdLockedOpq_ failed.\n");
                } else {
                    r = ScnFALSE;
                }
            }
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRender_jobFramebuffPropsPushLockedOpq_(STScnRenderOpq* opq, STScnRenderFbuffState* f, const STScnGpuFramebuffProps* pProps){
    ScnBOOL r = ScnFALSE;
    //add cmd
    STScnRenderFbuffProps props = STScnRenderFbuffProps_Zero;
    props.props = *pProps;
    props.ptr   = ScnBuffer_malloc(opq->fbPropsBuff, sizeof(STScnGpuFramebuffProps));
    if(props.ptr.ptr == NULL){
        printf("ERROR, ScnRender_jobFramebuffPropsPushLockedOpq_::ScnBuffer_malloc(STScnGpuFramebuffProps) failed.\n");
    } else if(!ScnRenderFbuffState_addProps(f, &props)){
        printf("ERROR, ScnRender_jobFramebuffPropsPushLockedOpq_::ScnRenderFbuffState_addProps failed.\n");
    } else {
        //add cmd
        STScnRenderCmd cmd;
        cmd.cmdId                       = ENScnRenderCmd_SetFramebuffProps;
        cmd.setFramebuffProps.offset    = props.ptr.idx;
        cmd.setFramebuffProps.viewport  = props.props.viewport;
        if(!ScnRender_addCmdLockedOpq_(opq, &cmd)){
            printf("ERROR, ScnRender_jobFramebuffPropsPushLockedOpq_::ScnRender_addCmdLockedOpq_ failed.\n");
        } else {
            //push fb props
            STScnGpuFramebuffProps* buffPtr = (STScnGpuFramebuffProps*)props.ptr.ptr;
            *buffPtr = props.props;
            //
            r = ScnTRUE;
        }
    }
    return r;
}

ScnBOOL ScnRender_jobFramebuffPropsPush(ScnRenderRef ref, const STScnGpuFramebuffProps pProps){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!opq->job.isActive){
        printf("ERROR, ScnRender_jobFramebuffPropsPush::not active.\n");
    } else if(opq->job.stackUse <= 0){
        printf("ERROR, ScnRender_jobFramebuffPropsPush::no active framebuffer.\n");
    } else {
        STScnRenderFbuffState* f = &opq->job.stack.arr[opq->job.stackUse - 1];
        //add cmd
        if(f->stacks.props.use <= 0){
            //program logic error (should be previously validated)
            printf("ERROR, ScnRender_jobFramebuffPropsPush::framebuff has not properties in stack.\n");
        } else {
            r = ScnRender_jobFramebuffPropsPushLockedOpq_(opq, f, &pProps);
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRender_jobFramebuffPropsOrthoPush(ScnRenderRef ref, const STScnAABBox3d ortho){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!opq->job.isActive){
        printf("ERROR, ScnRender_jobFramebuffPropsPush::not active.\n");
    } else if(opq->job.stackUse <= 0){
        printf("ERROR, ScnRender_jobFramebuffPropsPush::no active framebuffer.\n");
    } else {
        STScnRenderFbuffState* f = &opq->job.stack.arr[opq->job.stackUse - 1];
        //add cmd
        if(f->stacks.props.use <= 0){
            //program logic error (should be previously validated)
            printf("ERROR, ScnRender_jobFramebuffPropsPush::framebuff has not properties in stack.\n");
        } else {
            STScnGpuFramebuffProps propsPrev = f->stacks.props.arr[f->stacks.props.use - 1].props;
            propsPrev.ortho = ortho;
            r = ScnRender_jobFramebuffPropsPushLockedOpq_(opq, f, &propsPrev);
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRender_jobFramebuffPropsPop(ScnRenderRef ref){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!opq->job.isActive){
        printf("ERROR, ScnRender_jobFramebuffPropsPop::not active.\n");
    } else if(opq->job.stackUse <= 0){
        printf("ERROR, ScnRender_jobFramebuffPropsPop::no active framebuffer.\n");
    } else {
        STScnRenderFbuffState* f = &opq->job.stack.arr[opq->job.stackUse - 1];
        //add cmd
        if(f->stacks.props.use <= 1){
            printf("ERROR, ScnRender_jobFramebuffPropsPop::framebuff has not properties to pop.\n");
        } else {
            //add cmd
            const STScnRenderFbuffProps* propsPrev = &f->stacks.props.arr[f->stacks.props.use - 1];
            STScnRenderCmd cmd;
            cmd.cmdId                       = ENScnRenderCmd_SetFramebuffProps;
            cmd.setFramebuffProps.offset    = propsPrev->ptr.idx;
            cmd.setFramebuffProps.viewport  = propsPrev->props.viewport;
            if(!ScnRender_addCmdLockedOpq_(opq, &cmd)){
                printf("ERROR, ScnRender_jobFramebuffPropsPop::ScnRender_addCmdLockedOpq_ failed.\n");
            } else {
                r = ScnTRUE;
            }
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//job textures

ScnBOOL ScnRender_jobSetTextureLockedOpq_(STScnRenderOpq* opq, STScnRenderFbuffState* f, const ENScnGpuTextureIdx idx, ScnTextureRef tex){
    ScnBOOL r = ScnFALSE;
    if(ScnTexture_isSame(f->active.texs.refs[idx], tex)){
        //printf("Same texture, ignoring command.\n");
        r = ScnTRUE;
    } else if(!ScnRender_jobAddUsedObjLockedOpq_(opq, ENScnRenderJobObjType_Texture, (ScnObjRef*)&tex)){
        printf("ERROR, ScnRender_jobSetTexture::ScnRender_jobAddUsedObjLockedOpq_ failed.\n");
    } else {
        //add cmd
        STScnRenderCmd cmd;
        cmd.cmdId               = ENScnRenderCmd_SetTexture;
        cmd.setTexture.ref      = tex;
        cmd.setTexture.index    = idx;
        if(!ScnRender_addCmdLockedOpq_(opq, &cmd)){
            printf("ERROR, ScnRender_jobSetTexture::ScnRender_addCmdLockedOpq_ failed.\n");
        } else {
            f->active.texs.refs[idx] = tex; //do not retain
            ++f->active.texs.changesSeq;
            r = ScnTRUE;
        }
    }
    return r;
}

ScnBOOL ScnRender_jobSetTexture(ScnRenderRef ref, const ENScnGpuTextureIdx idx, ScnTextureRef tex){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!opq->job.isActive){
        printf("ERROR, ScnRender_jobSetTexture:: not active.\n");
    } else if(opq->job.stackUse <= 0){ //cmds-queues are per-framebuffer
        printf("ERROR, ScnRender_jobSetTexture::no active framebuffer.\n");
    } else if(idx < 0 || idx >= ENScnGpuTextureIdx_Count){
        printf("ERROR, ScnRender_jobSetTexture::idx-out-of-bounds.\n");
    } else {
        STScnRenderFbuffState* f = &opq->job.stack.arr[opq->job.stackUse - 1];
        r = ScnRender_jobSetTextureLockedOpq_(opq, f, idx, tex);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}


//job models

SC_INLN ScnBOOL ScnRender_job_addSetVertexBuffIfNecesaryLockedOpq_(STScnRenderOpq* opq, STScnRenderFbuffState* f, ScnVertexbuffRef vbuff){
    if(ScnVertexbuff_isSame(f->active.vbuff, vbuff)){
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
            return ScnFALSE;
        } else {
            f->active.vbuff = vbuff;
        }
    }
    return ScnTRUE;
}

#define ScnRender_jobAddCommandDrawVertsLockeOpq_(OPQ, FRAMEBUFF_STATE, DRAW_CMD, VERTEX_TYPE, VERTEX_TYPE_IDX) \
    if(ScnVertexbuffs_isNull((DRAW_CMD)->vbuffs)){ \
        printf("ERROR, ScnRender_jobModel2dAdd_addCommandsWithPropsLockedOpq_::cmd without ScnVertexbuffs.\n"); \
        r = ScnFALSE; \
    } else { \
        if(!ScnRender_job_addSetVertexBuffIfNecesaryLockedOpq_(OPQ, FRAMEBUFF_STATE, ScnVertexbuffs_getVertexBuff((DRAW_CMD)->vbuffs, VERTEX_TYPE))){ \
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

#define ScnRender_jobAddCommandDrawIndexesLockeOpq_(OPQ, FRAMEBUFF_STATE, DRAW_CMD, VERTEX_TYPE, VERTEX_TYPE_IDX) \
    if(ScnVertexbuffs_isNull((DRAW_CMD)->vbuffs)){ \
        printf("ERROR, ScnRender_jobModel2dAdd_addCommandsWithPropsLockedOpq_::cmd without ScnVertexbuffs.\n"); \
        r = ScnFALSE; \
    } else { \
        if(!ScnRender_job_addSetVertexBuffIfNecesaryLockedOpq_(OPQ, FRAMEBUFF_STATE, ScnVertexbuffs_getVertexBuff((DRAW_CMD)->vbuffs, VERTEX_TYPE))){ \
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

STScnAbsPtr ScnRender_jobModel2dAdd_addGpuModelPropsLockedOpq_(STScnRenderOpq* opq, STScnRenderFbuffState* f, const STScnGpuModelProps2d* const transform){
    STScnAbsPtr r = ScnBuffer_malloc(opq->mdlsPropsBuff, sizeof(STScnGpuModelProps2d));
    if(r.ptr == NULL){
        printf("ERROR, ScnRender_jobModel2dAdd_addCommandsWithTransformLockedOpq_::ScnBuffer_malloc failed.\n");
    } else {
        STScnRenderCmd cmd;
        cmd.cmdId = ENScnRenderCmd_SetTransformOffset;
        cmd.setTransformOffset.offset = r.idx;
        if(!ScnRender_addCmdLockedOpq_(opq, &cmd)){
            printf("ERROR, ScnRender_jobModel2dAdd_addCommandsWithTransformLockedOpq_::ScnRender_addCmdLockedOpq_ failed.\n");
            r.ptr = NULL;
        } else {
            STScnGpuModelProps2d* curProps = (STScnGpuModelProps2d*)r.ptr;
            *curProps = *transform;
            if(opq->gpuDevDesc.isTexFmtInfoRequired){
                curProps->texs.fmts[0] = ScnTexture_isNull(f->active.texs.refs[0]) ? ENScnBitmapColor_undef : ScnTexture_getImageProps(f->active.texs.refs[0], NULL).color;
                curProps->texs.fmts[1] = ScnTexture_isNull(f->active.texs.refs[1]) ? ENScnBitmapColor_undef : ScnTexture_getImageProps(f->active.texs.refs[0], NULL).color;
                curProps->texs.fmts[2] = ScnTexture_isNull(f->active.texs.refs[2]) ? ENScnBitmapColor_undef : ScnTexture_getImageProps(f->active.texs.refs[0], NULL).color;
            }
        }
    }
    return r;
}

ScnBOOL ScnRender_jobModel2dAdd_addCommandsWithTransformLockedOpq_(STScnRenderOpq* opq, STScnRenderFbuffState* f, const STScnGpuModelProps2d* const transform, const STScnModel2dCmd* const cmds, const ScnUI32 cmdsSz){
    ScnBOOL r = ScnFALSE;
    //add commands with this matrix (do not push it into the stack)
    STScnAbsPtr tNPtr = ScnRender_jobModel2dAdd_addGpuModelPropsLockedOpq_(opq, f, transform);
    if(tNPtr.ptr == NULL){
        printf("ERROR, ScnRender_jobModel2dAdd_addGpuModelPropsLockedOpq_::ScnBuffer_malloc failed.\n");
    } else {
        ScnUI32 texChangesVer = f->active.texs.changesSeq;
        r = ScnTRUE;
        //add model commands
        {
            const STScnModel2dCmd* c = cmds;
            const STScnModel2dCmd* cAfterEnd = c + cmdsSz;
            while(r && c < cAfterEnd){
                switch (c->type) {
                    case ENScnModelDrawCmdType_Undef:
                        //nop
                        break;
                    case ENScnModelDrawCmdType_2Dv0:
                        //draw command
                        ScnRender_jobAddCommandDrawVertsLockeOpq_(opq, f, c, ENScnVertexType_2DColor, 0);
                        break;
                    case ENScnModelDrawCmdType_2Dv1:
                        //sync textures
                        if(!ScnRender_jobSetTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_0, c->texs[0])){
                            r = ScnFALSE;
                        }
                        //inject new obj-props if textures changed and their information is required by the device
                        if(opq->gpuDevDesc.isTexFmtInfoRequired && texChangesVer != f->active.texs.changesSeq && NULL == (tNPtr = ScnRender_jobModel2dAdd_addGpuModelPropsLockedOpq_(opq, f, transform)).ptr){
                            printf("ERROR, ScnRender_jobModel2dAdd_addCommandsWithTransformLockedOpq_::ScnBuffer_malloc failed.\n");
                            r = ScnFALSE;
                            break;
                        } else {
                            texChangesVer = f->active.texs.changesSeq;
                        }
                        //draw command
                        ScnRender_jobAddCommandDrawVertsLockeOpq_(opq, f, c, ENScnVertexType_2DTex, 1);
                        break;
                    case ENScnModelDrawCmdType_2Dv2:
                        //sync textures
                        if(!ScnRender_jobSetTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_0, c->texs[0])){
                            r = ScnFALSE;
                        }
                        if(!ScnRender_jobSetTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_1, c->texs[1])){
                            r = ScnFALSE;
                        }
                        //inject new obj-props if textures changed and their information is required by the device
                        if(opq->gpuDevDesc.isTexFmtInfoRequired && texChangesVer != f->active.texs.changesSeq && NULL == (tNPtr = ScnRender_jobModel2dAdd_addGpuModelPropsLockedOpq_(opq, f, transform)).ptr){
                            printf("ERROR, ScnRender_jobModel2dAdd_addCommandsWithTransformLockedOpq_::ScnBuffer_malloc failed.\n");
                            r = ScnFALSE;
                            break;
                        } else {
                            texChangesVer = f->active.texs.changesSeq;
                        }
                        //draw command
                        ScnRender_jobAddCommandDrawVertsLockeOpq_(opq, f, c, ENScnVertexType_2DTex2, 2);
                        break;
                    case ENScnModelDrawCmdType_2Dv3:
                        //sync textures
                        if(!ScnRender_jobSetTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_0, c->texs[0])){
                            r = ScnFALSE;
                        }
                        if(!ScnRender_jobSetTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_1, c->texs[1])){
                            r = ScnFALSE;
                        }
                        if(!ScnRender_jobSetTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_2, c->texs[2])){
                            r = ScnFALSE;
                        }
                        //inject new obj-props if textures changed and their information is required by the device
                        if(opq->gpuDevDesc.isTexFmtInfoRequired && texChangesVer != f->active.texs.changesSeq && NULL == (tNPtr = ScnRender_jobModel2dAdd_addGpuModelPropsLockedOpq_(opq, f, transform)).ptr){
                            printf("ERROR, ScnRender_jobModel2dAdd_addCommandsWithTransformLockedOpq_::ScnBuffer_malloc failed.\n");
                            r = ScnFALSE;
                            break;
                        } else {
                            texChangesVer = f->active.texs.changesSeq;
                        }
                        //draw command
                        ScnRender_jobAddCommandDrawVertsLockeOpq_(opq, f, c, ENScnVertexType_2DTex3, 3);
                        break;
                        //
                    case ENScnModelDrawCmdType_2Di0:
                        //draw command
                        ScnRender_jobAddCommandDrawIndexesLockeOpq_(opq, f, c, ENScnVertexType_2DColor, 0);
                        break;
                    case ENScnModelDrawCmdType_2Di1:
                        //sync textures
                        if(!ScnRender_jobSetTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_0, c->texs[0])){
                            r = ScnFALSE;
                        }
                        //inject new obj-props if textures changed and their information is required by the device
                        if(opq->gpuDevDesc.isTexFmtInfoRequired && texChangesVer != f->active.texs.changesSeq && NULL == (tNPtr = ScnRender_jobModel2dAdd_addGpuModelPropsLockedOpq_(opq, f, transform)).ptr){
                            printf("ERROR, ScnRender_jobModel2dAdd_addCommandsWithTransformLockedOpq_::ScnBuffer_malloc failed.\n");
                            r = ScnFALSE;
                            break;
                        } else {
                            texChangesVer = f->active.texs.changesSeq;
                        }
                        //draw command
                        ScnRender_jobAddCommandDrawIndexesLockeOpq_(opq, f, c, ENScnVertexType_2DTex, 1);
                        break;
                    case ENScnModelDrawCmdType_2Di2:
                        //sync textures
                        if(!ScnRender_jobSetTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_0, c->texs[0])){
                            r = ScnFALSE;
                        }
                        if(!ScnRender_jobSetTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_1, c->texs[1])){
                            r = ScnFALSE;
                        }
                        //inject new obj-props if textures changed and their information is required by the device
                        if(opq->gpuDevDesc.isTexFmtInfoRequired && texChangesVer != f->active.texs.changesSeq && NULL == (tNPtr = ScnRender_jobModel2dAdd_addGpuModelPropsLockedOpq_(opq, f, transform)).ptr){
                            printf("ERROR, ScnRender_jobModel2dAdd_addCommandsWithTransformLockedOpq_::ScnBuffer_malloc failed.\n");
                            r = ScnFALSE;
                            break;
                        } else {
                            texChangesVer = f->active.texs.changesSeq;
                        }
                        //draw command
                        ScnRender_jobAddCommandDrawIndexesLockeOpq_(opq, f, c, ENScnVertexType_2DTex2, 2);
                        break;
                    case ENScnModelDrawCmdType_2Di3:
                        //sync textures
                        if(!ScnRender_jobSetTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_0, c->texs[0])){
                            r = ScnFALSE;
                        }
                        if(!ScnRender_jobSetTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_1, c->texs[1])){
                            r = ScnFALSE;
                        }
                        if(!ScnRender_jobSetTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_2, c->texs[2])){
                            r = ScnFALSE;
                        }
                        //inject new obj-props if textures changed and their information is required by the device
                        if(opq->gpuDevDesc.isTexFmtInfoRequired && texChangesVer != f->active.texs.changesSeq && NULL == (tNPtr = ScnRender_jobModel2dAdd_addGpuModelPropsLockedOpq_(opq, f, transform)).ptr){
                            printf("ERROR, ScnRender_jobModel2dAdd_addCommandsWithTransformLockedOpq_::ScnBuffer_malloc failed.\n");
                            r = ScnFALSE;
                            break;
                        } else {
                            texChangesVer = f->active.texs.changesSeq;
                        }
                        //draw command
                        ScnRender_jobAddCommandDrawIndexesLockeOpq_(opq, f, c, ENScnVertexType_2DTex3, 3);
                        break;
                    default:
                        SCN_ASSERT(ScnFALSE) //missing implementation
                        break;
                }
                //ToDo: cmds to activate textures
                ++c;
            }
        }
    }
    return r;
}
    
ScnBOOL ScnRender_jobModel2dAdd_addCommandsWithPropsLockedOpq_(void* data, const STScnGpuModelProps2d* const props, const STScnModel2dCmd* const cmds, const ScnUI32 cmdsSz){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)data;
    if(opq != NULL && props != NULL){
        if(!opq->job.isActive){
            printf("ERROR, ScnRender_jobModel2dAdd_addCommandsWithPropsLockedOpq_::not active.\n");
        } else if(opq->job.stackUse <= 0){
            printf("ERROR, ScnRender_jobModel2dAdd_addCommandsWithPropsLockedOpq_::no active framebuffer.\n");
        } else {
            STScnRenderFbuffState* f = &opq->job.stack.arr[opq->job.stackUse - 1];
            if(f->stacks.transforms.use < 1){
                printf("ERROR, ScnRender_jobModel2dAdd_addCommandsWithPropsLockedOpq_::missing parent transform.\n");
            } else {
                SCN_ASSERT(!ScnBuffer_isNull(opq->fbPropsBuff)) //program logic error
                SCN_ASSERT(!ScnBuffer_isNull(opq->mdlsPropsBuff)) //program logic error
                if(!ScnRender_jobModel2dAdd_addCommandsWithTransformLockedOpq_(opq, f, props, cmds, cmdsSz)){
                    printf("ERROR, ScnRender_jobModel2dAdd_addCommandsWithPropsLockedOpq_::ScnRender_jobModel2dAdd_addCommandsWithTransformLockedOpq_ failed.\n");
                } else {
                    r = ScnTRUE;
                }
            }
        }
    }
    return r;
}

//job node (scene tree)

ScnBOOL ScnRender_jobNode2dPush(ScnRenderRef ref, ScnNode2dRef node){
    return ScnRender_jobNode2dPropsPush(ref, ScnNode2d_getProps(node));
}

ScnBOOL ScnRender_jobNode2dPropsPush(ScnRenderRef ref, const STScnNode2dProps nodeProps){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!opq->job.isActive){
        printf("ERROR, ScnRender_jobNode2dPropsPush::not active.\n");
    } else if(opq->job.stackUse <= 0){
        printf("ERROR, ScnRender_jobNode2dPropsPush::no active framebuffer.\n");
    } else {
        STScnRenderFbuffState* f = &opq->job.stack.arr[opq->job.stackUse - 1];
        if(f->stacks.transforms.use < 1){
            printf("ERROR, ScnRender_jobNode2dPropsPush::missing parent transform.\n");
        } else {
            SCN_ASSERT(!ScnBuffer_isNull(opq->fbPropsBuff)) //program logic error
            SCN_ASSERT(!ScnBuffer_isNull(opq->mdlsPropsBuff)) //program logic error
            const STScnGpuModelProps2d t = ScnNode2dProps_toGpuTransform(&nodeProps);
            const STScnGpuModelProps2d tPrnt = f->stacks.transforms.arr[f->stacks.transforms.use - 1];
            const STScnGpuModelProps2d tN = ScnGpuModelProps2d_multiply(&tPrnt, &t);
            if(!ScnRenderFbuffState_addTransform(f, &tN)){
                printf("ERROR, ScnRender_jobNode2dPropsPush::ScnRenderFbuffState_addTransform failed.\n");
            } else {
                //ToDo: add render props to buffer
                r = ScnTRUE;
            }
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRender_jobNode2dPop(ScnRenderRef ref){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!opq->job.isActive){
        printf("ERROR, ScnRender_jobNode2dPop::not active.\n");
    } else if(opq->job.stackUse <= 0){
        printf("ERROR, ScnRender_jobNode2dPop::no framebuffer is active.\n");
    } else {
        SCN_ASSERT(!ScnBuffer_isNull(opq->fbPropsBuff)) //program logic error
        SCN_ASSERT(!ScnBuffer_isNull(opq->mdlsPropsBuff)) //program logic error
        STScnRenderFbuffState* f = &opq->job.stack.arr[opq->job.stackUse - 1];
        if(f->stacks.transforms.use < 2){
            printf("ERROR, ScnRender_jobNode2dPop::nothing to pop.\n");
        } else {
            f->stacks.transforms.use--;
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//job models


ScnBOOL ScnRender_jobModel2dAddWithNode(ScnRenderRef ref, ScnModel2dRef model, ScnNode2dRef node){ //equivalent to jobNode2dPush() + jobModelAdd() + jobNodePropsPop()
    return !ScnModel2d_isNull(model) && !ScnNode2d_isNull(node) ? ScnRender_jobModel2dAddWithNodeProps(ref, model, ScnNode2d_getProps(node)) : ScnFALSE;
}

ScnBOOL ScnRender_jobModel2dAddWithNodeProps(ScnRenderRef ref, ScnModel2dRef model, const STScnNode2dProps nodeProps){ //equivalent to jobNodePropsPush() + jobModelAdd() + jobNodePropsPop()
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!opq->job.isActive){
        printf("ERROR, ScnRender_jobModel2dAddWithNodeProps::not active.\n");
    } else if(opq->job.stackUse <= 0){
        printf("ERROR, ScnRender_jobModel2dAddWithNodeProps::no active framebuffer.\n");
    } else {
        STScnRenderFbuffState* f = &opq->job.stack.arr[opq->job.stackUse - 1];
        if(f->stacks.transforms.use < 1){
            printf("ERROR, ScnRender_jobModel2dAddWithNodeProps::missing parent transform.\n");
        } else {
            SCN_ASSERT(!ScnBuffer_isNull(opq->fbPropsBuff)) //program logic error
            SCN_ASSERT(!ScnBuffer_isNull(opq->mdlsPropsBuff)) //program logic error
            const STScnGpuModelProps2d t = ScnNode2dProps_toGpuTransform(&nodeProps);
            const STScnGpuModelProps2d tPrnt = f->stacks.transforms.arr[f->stacks.transforms.use - 1];
            const STScnGpuModelProps2d tN = ScnGpuModelProps2d_multiply(&tPrnt, &t);
            STScnModel2dPushItf itf = STScnModel2dPushItf_Zero;
            itf.addCommandsWithProps = ScnRender_jobModel2dAdd_addCommandsWithPropsLockedOpq_;
            if(!ScnModel2d_sendRenderCmds(model, &tN, &itf, opq)){
                printf("ERROR, ScnRender_jobModel2dAddWithNodeProps::ScnModel2d_sendRenderCmds failed.\n");
            } else {
                r = ScnTRUE;
            }
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRender_jobModel2dAdd(ScnRenderRef ref, ScnModel2dRef model){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!opq->job.isActive){
        printf("ERROR, ScnRender_jobModel2dAdd::not active.\n");
    } else if(opq->job.stackUse <= 0){
        printf("ERROR, ScnRender_jobModel2dAdd::no framebuffer is active.\n");
    } else {
        SCN_ASSERT(!ScnBuffer_isNull(opq->fbPropsBuff)) //program logic error
        SCN_ASSERT(!ScnBuffer_isNull(opq->mdlsPropsBuff)) //program logic error
        STScnRenderFbuffState* f = &opq->job.stack.arr[opq->job.stackUse - 1];
        if(f->stacks.transforms.use < 1){
            printf("ERROR, ScnRender_jobModel2dAdd::render props unavailable.\n");
        } else {
            SCN_ASSERT(!ScnBuffer_isNull(opq->fbPropsBuff)) //program logic error
            SCN_ASSERT(!ScnBuffer_isNull(opq->mdlsPropsBuff)) //program logic error
            const STScnGpuModelProps2d tPrnt = f->stacks.transforms.arr[f->stacks.transforms.use - 1];
            STScnModel2dPushItf itf = STScnModel2dPushItf_Zero;
            itf.addCommandsWithProps = ScnRender_jobModel2dAdd_addCommandsWithPropsLockedOpq_;
            if(!ScnModel2d_sendRenderCmds(model, &tPrnt, &itf, opq)){
                printf("ERROR, ScnRender_jobFramebuffPop::ScnModel2d_sendRenderCmds failed.\n");
            } else {
                r = ScnTRUE;
            }
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

ScnBOOL ScnRender_jobAddUsedObjLockedOpq_(STScnRenderOpq* opq, const ENScnRenderJobObjType type, ScnObjRef* objRef){
    SCN_ASSERT(!ScnObjRef_isNull(*objRef)) //program logic error (validate becfore reaching this point)
    ScnBOOL r = ScnFALSE;
    STScnRenderJobObj usedObj;
    usedObj.type    = type;
    usedObj.objRef  = *objRef;
    const ScnSI32 iFnd = ScnArraySorted_indexOf(&opq->job.objs, &usedObj);
    if(iFnd >= 0){
        //already added;
        r = ScnTRUE;
    } else {
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
