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

//STScnRenderOpq

typedef struct STScnRenderOpq_ {
    STScnContextRef         ctx;
    STScnMutexRef           mutex;
    ScnUI32                 ammRenderSlots;
    //api
    struct {
        STScnApiItf         itf;
        void*               itfParam;
        void*               data;
    } api;
    STScnGpuDeviceRef       gpuDev;
    STScnBufferRef          viewPropsBuff;  //buffer with viewport properties
    STScnBufferRef          nodesPropsBuff; //buffer with models
    STScnVertexbuffRef      vbuffs;         //buffers with vertices and indices
    //job
    struct {
        //nodes
        struct {
            ScnUI16        iDepth;
            ScnUI32        ammOpen;
            ScnUI32        ammPoppedAtEnd; //pop-optimization, ammount of nodes already closed at the end of the array
            ScnArrayStruct(arr, STScnRenderNode);
        } nodes;
        ScnArrayStruct(cmds, STScnRenderCmd);
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
    opq->mutex = ScnContext_mutex_alloc(opq->ctx);
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
        ScnArray_init(opq->ctx, &opq->job.nodes.arr, 256, 256, STScnRenderNode)
        ScnArray_init(opq->ctx, &opq->job.cmds, 256, 256, STScnRenderCmd)
    }
}

void ScnRender_destroyOpq(void* obj){
    STScnRenderOpq* opq = (STScnRenderOpq*)obj;
    //job
    {
        ScnArray_destroy(opq->ctx, &opq->job.nodes.arr);
        ScnArray_destroy(opq->ctx, &opq->job.cmds);
    }
    if(!ScnBuffer_isNull(opq->viewPropsBuff)){
        ScnBuffer_release(&opq->viewPropsBuff);
        ScnBuffer_null(&opq->viewPropsBuff);
    }
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
    {
        if(!ScnGpuDevice_isNull(opq->gpuDev)){
            ScnGpuDevice_release(&opq->gpuDev);
            ScnGpuDevice_null(&opq->gpuDev);
        }
    }
    //api
    {
        if(opq->api.data != NULL){
            opq->api.data = NULL;
        }
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
                /*STScnBufferRef viewPropsBuff =
                if(!ScnBuffer_isNull(opq->viewPropsBuff)){
                    ScnBuffer_release(&opq->viewPropsBuff);
                    ScnBuffer_null(&opq->viewPropsBuff);
                }
                if(!ScnBuffer_isNull(opq->nodesPropsBuff)){
                    ScnBuffer_release(&opq->nodesPropsBuff);
                    ScnBuffer_null(&opq->nodesPropsBuff);
                }*/
                //vbuffs
                {
                    STScnVertexbuffsRef vbuffs = ScnRender_allocVertexbuffsLockedOpq_(opq, ammRenderSlots);
                    if(!ScnVertexbuffs_isNull(vbuffs)){
                        ScnVertexbuffs_set(&opq->vbuffs, vbuffs);
                        r = ScnTRUE;
                    }
                }
            }
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
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
                STScnGpuBufferCfg cfg    = STScnGpuBufferCfg_Zero;
                cfg.mem.idxsAlign       = itmSz;
                cfg.mem.sizeAlign       = 256;
                cfg.mem.sizeInitial     = 0;
                //calculate sizePerBlock
                {
                    ScnUI32 idxExtra = 0;
                    cfg.mem.sizePerBlock = ((ammPerBock * cfg.mem.idxsAlign) + cfg.mem.sizeAlign - 1) / cfg.mem.sizeAlign * cfg.mem.sizeAlign;
                    idxExtra = cfg.mem.sizePerBlock % cfg.mem.idxsAlign;
                    if(idxExtra > 0){
                        cfg.mem.sizePerBlock *= (cfg.mem.idxsAlign - idxExtra);
                    }
                    SCN_ASSERT((cfg.mem.sizePerBlock % cfg.mem.idxsAlign) == 0)
                    SCN_ASSERT((cfg.mem.sizePerBlock % cfg.mem.sizeAlign) == 0)
                }
                vBuffs[i] = ScnBuffer_alloc(opq->ctx);
                if(ScnBuffer_isNull(vBuffs[i])){
                    //error
                    SCN_ASSERT(ScnFALSE)
                    r = ScnFALSE;
                } else if(!ScnBuffer_prepare(vBuffs[i], opq->gpuDev, ammRenderSlots, &cfg)){
                    //error
                    SCN_ASSERT(ScnFALSE)
                    r = ScnFALSE;
                }
            }
            //indices buffer
            if(r){
                itmSz = sizeof(STScnVertexIdx);
                ammPerBock = 2048;
                STScnGpuBufferCfg cfg    = STScnGpuBufferCfg_Zero;
                cfg.mem.idxsAlign       = itmSz;
                cfg.mem.sizeAlign       = 256;
                cfg.mem.sizeInitial     = 0;
                //calculate sizePerBlock
                {
                    ScnUI32 idxExtra = 0;
                    cfg.mem.sizePerBlock = ((ammPerBock * cfg.mem.idxsAlign) + cfg.mem.sizeAlign - 1) / cfg.mem.sizeAlign * cfg.mem.sizeAlign;
                    idxExtra = cfg.mem.sizePerBlock % cfg.mem.idxsAlign;
                    if(idxExtra > 0){
                        cfg.mem.sizePerBlock *= (cfg.mem.idxsAlign - idxExtra);
                    }
                    SCN_ASSERT((cfg.mem.sizePerBlock % cfg.mem.idxsAlign) == 0)
                    SCN_ASSERT((cfg.mem.sizePerBlock % cfg.mem.sizeAlign) == 0)
                }
                iBuffs[i] = ScnBuffer_alloc(opq->ctx);
                if(ScnBuffer_isNull(iBuffs[i])){
                    //error
                    SCN_ASSERT(ScnFALSE)
                    r = ScnFALSE;
                } else if(!ScnBuffer_prepare(iBuffs[i], opq->gpuDev, ammRenderSlots, &cfg)){
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
            } SCN_ASSERT((cfg.szPerRecord % 4) == 0)
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
                    if(!ScnGpuBuffer_isNull(idxsBuff)){
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
    {
        opq->job.nodes.iDepth = 0;
        opq->job.nodes.ammOpen = 0;
        opq->job.nodes.ammPoppedAtEnd = 0;
        ScnArray_empty(&opq->job.nodes.arr);
        ScnArray_empty(&opq->job.cmds);
        r = ScnTRUE;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}
    
ScnBOOL ScnRender_jobEnd(STScnRenderRef ref){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        //implicitly close remaining open nodes
        if(opq->job.nodes.arr.use > opq->job.nodes.ammPoppedAtEnd){
            STScnRenderNode* nFist = opq->job.nodes.arr.arr;
            STScnRenderNode* n = nFist + opq->job.nodes.arr.use - 1 - opq->job.nodes.ammPoppedAtEnd;
            const STScnRenderNode* nAfterLast = nFist + opq->job.nodes.arr.use;
            while(opq->job.nodes.ammOpen > 0 && n >= nFist) {
                if(!n->isPopped){
                    SCN_ASSERT(n->cmds.start <= opq->job.cmds.use)
                    n->isPopped     = ScnTRUE;
                    n->underCount   = (ScnUI32)(nAfterLast - n);
                    n->cmds.size    = (ScnUI32)(opq->job.cmds.use - n->cmds.start);
                    //
                    --opq->job.nodes.ammOpen;
                }
                ++opq->job.nodes.ammPoppedAtEnd;
                --n;
            }
        }
        SCN_ASSERT(opq->job.nodes.ammOpen == 0)
        SCN_ASSERT(opq->job.nodes.ammPoppedAtEnd == opq->job.nodes.arr.use);
        r = (opq->job.nodes.ammOpen == 0 && opq->job.nodes.ammPoppedAtEnd == opq->job.nodes.arr.use);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//job nodes

void ScnRender_nodePush(STScnRenderRef ref, const STScnTransform* tform){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    if(tform != NULL){
        ScnMutex_lock(opq->mutex);
        {
            STScnRenderNode n = STScnRenderNode_Zero;
            n.cmds.start    = opq->job.cmds.use;
            n.iDepth        = opq->job.nodes.iDepth++;
            n.tform         = *tform;
            ScnArray_addValue(opq->ctx, &opq->job.nodes.arr, n, STScnRenderNode);
            //
            ++opq->job.nodes.ammOpen;
            opq->job.nodes.ammPoppedAtEnd = 0;
        }
        ScnMutex_unlock(opq->mutex);
    }
}
    
ScnBOOL ScnRender_nodePop(STScnRenderRef ref){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    SCN_ASSERT(opq->job.nodes.ammOpen > 0 && opq->job.nodes.arr.use > opq->job.nodes.ammPoppedAtEnd) //popping beyond limit
    if(opq->job.nodes.ammOpen > 0 && opq->job.nodes.arr.use > opq->job.nodes.ammPoppedAtEnd){
        STScnRenderNode* nFist = opq->job.nodes.arr.arr;
        STScnRenderNode* n = nFist + opq->job.nodes.arr.use - 1 - opq->job.nodes.ammPoppedAtEnd;
        const STScnRenderNode* nAfterLast = nFist + opq->job.nodes.arr.use;
        while(opq->job.nodes.ammOpen > 0 && n >= nFist) {
            ++opq->job.nodes.ammPoppedAtEnd;
            if(!n->isPopped){
                SCN_ASSERT(n->cmds.start <= opq->job.cmds.use)
                n->isPopped     = ScnTRUE;
                n->underCount   = (ScnUI32)(nAfterLast - n);
                n->cmds.size    = (ScnUI32)(opq->job.cmds.use - n->cmds.start);
                //
                --opq->job.nodes.ammOpen;
                r = ScnTRUE;
                break;
            }
            --n;
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
        //vbuffs
        if(r && !ScnVertexbuffs_isNull(opq->vbuffs) && !ScnVertexbuffs_prepareNextRenderSlot(opq->vbuffs)){
            printf("ERROR, ScnRender_prepareNextRenderSlot::ScnVertexbuffs_prepareNextRenderSlot failed.\n");
            r = ScnFALSE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}
