//
//  ScnRender.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/scene/ScnRender.h"
#include "ixrender/core/ScnArray.h"
#include "ixrender/core/ScnArraySorted.h"
#include "ixrender/gpu/ScnGpuDataType.h"

//STScnRenderBuff

#define STScnRenderBuff_Zero { 0, STScnObjRef_Zero }

typedef struct STScnRenderBuff_ {
    ScnUI32             uid;    //internal unique-id
    STScnGpuBufferRef   buff;   //buffer
} STScnRenderBuff;

void ScnRenderBuff_release(STScnRenderBuff* obj);

ScnBOOL ScnCompare_NBScnRenderBuff(const ENScnCompareMode mode, const void* data1, const void* data2, const ScnUI32 dataSz){
    SCN_ASSERT(dataSz == sizeof(STScnRenderBuff))
    if(dataSz == sizeof(STScnRenderBuff)){
        const STScnRenderBuff* d1 = (STScnRenderBuff*)data1;
        const STScnRenderBuff* d2 = (STScnRenderBuff*)data2;
        switch (mode) {
            case ENScnCompareMode_Equal: return (d1->uid == d2->uid);
            case ENScnCompareMode_Lower: return (d1->uid < d2->uid);
            case ENScnCompareMode_LowerOrEqual: return (d1->uid <= d2->uid);
            case ENScnCompareMode_Greater: return (d1->uid > d2->uid);
            case ENScnCompareMode_GreaterOrEqual: return (d1->uid >= d2->uid);
            default: SCN_ASSERT(Scn_FALSE) break;
        }
    }
    return Scn_FALSE;
}

//STScnRenderVertexBuff

#define STScnRenderVertexBuff_Zero { 0, STScnObjRef_Zero }

typedef struct STScnRenderVertexBuff_ {
    ScnUI32                 uid;    //internal unique-id
    STScnGpuVertexbuffRef   buff;   //buffer
} STScnRenderVertexBuff;

void ScnRenderVertexBuff_release(STScnRenderVertexBuff* obj);

ScnBOOL ScnCompare_NBScnRenderVertexBuff(const ENScnCompareMode mode, const void* data1, const void* data2, const ScnUI32 dataSz){
    SCN_ASSERT(dataSz == sizeof(STScnRenderVertexBuff))
    if(dataSz == sizeof(STScnRenderVertexBuff)){
        const STScnRenderVertexBuff* d1 = (STScnRenderVertexBuff*)data1;
        const STScnRenderVertexBuff* d2 = (STScnRenderVertexBuff*)data2;
        switch (mode) {
            case ENScnCompareMode_Equal: return (d1->uid == d2->uid);
            case ENScnCompareMode_Lower: return (d1->uid < d2->uid);
            case ENScnCompareMode_LowerOrEqual: return (d1->uid <= d2->uid);
            case ENScnCompareMode_Greater: return (d1->uid > d2->uid);
            case ENScnCompareMode_GreaterOrEqual: return (d1->uid >= d2->uid);
            default: SCN_ASSERT(Scn_FALSE) break;
        }
    }
    return Scn_FALSE;
}

//STScnRenderOpq

typedef struct STScnRenderOpq_ {
    STScnContextRef         ctx;
    STScnMutexRef           mutex;
    //api
    struct {
        STScnRenderApiItf   itf;
        void*               itfParam;
        void*               data;
    } api;
    //buffs
    struct {
        ScnUI32             iSeq;   //uid seq
        ScnArraySortedStruct(arr, STScnRenderBuff);
    } buffs;
    //vertexBuffs
    struct {
        ScnUI32             iSeq;   //uid seq
        STScnVertexbuffsRef def;    //default (vertices and indices allocator)
        ScnArraySortedStruct(arr, STScnRenderVertexBuff);
    } vertexBuffs;
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
    //buffers
    {
        ScnArraySorted_init(opq->ctx, &opq->buffs.arr, 0, 16, STScnRenderBuff, ScnCompare_NBScnRenderBuff);
    }
    //vertexBuffers
    {
        ScnArraySorted_init(opq->ctx, &opq->vertexBuffs.arr, 0, 16, STScnRenderVertexBuff, ScnCompare_NBScnRenderVertexBuff);
        opq->vertexBuffs.def = ScnVertexbuffs_alloc(opq->ctx); //default (vertices and indices allocator)
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
    //vertexBuffs
    {
        //default
        if(!ScnVertexbuffs_isNull(opq->vertexBuffs.def)){
            ScnVertexbuffs_release(&opq->vertexBuffs.def);
            ScnVertexbuffs_null(&opq->vertexBuffs.def);
        }
        //array
        {
            STScnRenderVertexBuff* b = opq->vertexBuffs.arr.arr;
            const STScnRenderVertexBuff* bAfterEnd = b + opq->vertexBuffs.arr.use;
            while(b < bAfterEnd){
                ScnRenderVertexBuff_release(b);
                ++b;
            }
            ScnArraySorted_empty(&opq->vertexBuffs.arr);
            ScnArraySorted_destroy(opq->ctx, &opq->vertexBuffs.arr);
            opq->vertexBuffs.iSeq = 0;
        }
    }
    //buffs
    {
        STScnRenderBuff* b = opq->buffs.arr.arr;
        const STScnRenderBuff* bAfterEnd = b + opq->buffs.arr.use;
        while(b < bAfterEnd){
            ScnRenderBuff_release(b);
            ++b;
        }
        ScnArraySorted_empty(&opq->buffs.arr);
        ScnArraySorted_destroy(opq->ctx, &opq->buffs.arr);
        opq->buffs.iSeq = 0;
    }
    //api
    {
        if(opq->api.data != NULL){
            opq->api.data = NULL;
        }
        ScnMemory_setZeroSt(opq->api.itf, STScnRenderApiItf);
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

ScnBOOL ScnRender_createVertexbuffsLockedOpq_(STScnRenderOpq* opq, STScnVertexbuffsRef* dst);

ScnBOOL ScnRender_prepare(STScnRenderRef ref, const STScnRenderApiItf* itf, void* itfParam){
    ScnBOOL r = Scn_FALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->buffs.arr.use == 0 && itf != NULL){
        r = Scn_TRUE;
        //
        opq->api.itf = *itf;
        opq->api.itfParam = itfParam;
        //initial bufffers
        if(!ScnRender_createVertexbuffsLockedOpq_(opq, &opq->vertexBuffs.def)){
            r = Scn_FALSE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//Vertices

STScnVertexbuffsRef ScnRender_getDefaultVertexbuffs(STScnRenderRef ref){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->vertexBuffs.def;
}

ScnBOOL ScnRender_createVertexbuffs(STScnRenderRef ref, STScnVertexbuffsRef* dst){
    ScnBOOL r = Scn_FALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        r = ScnRender_createVertexbuffsLockedOpq_(opq, dst);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRender_createVertexbuffsLockedOpq_(STScnRenderOpq* opq, STScnVertexbuffsRef* dst){
    ScnBOOL r = Scn_TRUE;
    const ScnUI32 ammBuffsBef = opq->buffs.arr.use;
    const ScnUI32 ammVertsBuffsBef = opq->vertexBuffs.arr.use;
    //initial bufffers
    if(r){
        ScnSI32 i; for(i = 0; i <= ENScnVertexType_Count; i++){
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
                case ENScnVertexType_Count:
                    itmSz = sizeof(STScnVertexIdx);
                    ammPerBock = 2048;
                    break;
                default: SCN_ASSERT(Scn_FALSE); r = Scn_FALSE; break;
            }
            if(r){
                STScnGpuBufferCfg cfg    = STScnGpuBufferCfg_Zero;
                cfg.type                = (ENScnVertexType)i;
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
                STScnRenderBuff b = STScnRenderBuff_Zero;
                b.buff = ScnGpuBuffer_alloc(opq->ctx);
                if(!ScnGpuBuffer_prepare(b.buff, &cfg, &opq->api.itf.buff, opq->api.itfParam)){
                    //error
                    SCN_ASSERT(Scn_FALSE)
                    r = Scn_FALSE;
                } else {
                    b.uid = ++opq->buffs.iSeq;
                    ScnArraySorted_addPtr(opq->ctx, &opq->buffs.arr, &b, STScnRenderBuff);
                }
            }
        }
    }
    //initial vertexBuffers
    if(r){
        STScnGpuVertexbuffRef vbs[ENScnVertexType_Count];
        memset(vbs, 0, sizeof(vbs));
        {
            ScnSI32 i; for(i = 0; i < ENScnVertexType_Count; i++){
                STScnGpuVertexbuffCfg cfg = STScnGpuVertexbuffCfg_Zero;
                STScnGpuBufferRef vertexBuff = opq->buffs.arr.arr[ammBuffsBef + i].buff;
                STScnGpuBufferRef idxsBuff = opq->buffs.arr.arr[ammBuffsBef + ENScnVertexType_Count].buff;
                //size
                switch(i){
                    case ENScnVertexType_Tex3: cfg.szPerRecord = sizeof(STScnVertexTex3); break;
                    case ENScnVertexType_Tex2: cfg.szPerRecord = sizeof(STScnVertexTex2); break;
                    case ENScnVertexType_Tex: cfg.szPerRecord = sizeof(STScnVertexTex); break;
                    case ENScnVertexType_Color: cfg.szPerRecord = sizeof(STScnVertex); break;
                    default: SCN_ASSERT(Scn_FALSE) r = Scn_FALSE; break;
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
                    STScnRenderVertexBuff b = STScnRenderVertexBuff_Zero;
                    b.buff = ScnGpuVertexbuff_alloc(opq->ctx);
                    if(!ScnGpuVertexbuff_prepare(b.buff, &cfg, vertexBuff, idxsBuff, &opq->api.itf.vertexBuff, opq->api.itfParam)){
                        //error
                        SCN_ASSERT(Scn_FALSE)
                        r = Scn_FALSE;
                    } else {
                        vbs[i] = b.buff;
                        b.uid = ++opq->vertexBuffs.iSeq;
                        ScnArraySorted_addPtr(opq->ctx, &opq->vertexBuffs.arr, &b, STScnRenderVertexBuff);
                    }
                }
            }
        }
        //
        if(r){
            STScnVertexbuffsRef vbObj = ScnVertexbuffs_alloc(opq->ctx);
            if(ScnVertexbuffs_prepare(vbObj, vbs, sizeof(vbs) / sizeof(vbs[0]))){
                ScnVertexbuffs_set(dst, vbObj);
            } else {
                r = Scn_FALSE;
            }
            ScnVertexbuffs_release(&vbObj);
            ScnVertexbuffs_null(&vbObj);
        }
    }
    //revert
    if(!r){
        //vertexBuffs
        if(ammVertsBuffsBef < opq->vertexBuffs.arr.use){
            STScnRenderVertexBuff* bStart = opq->vertexBuffs.arr.arr;
            const STScnRenderVertexBuff* bAfterEnd = bStart + opq->vertexBuffs.arr.use;
            STScnRenderVertexBuff* b = bStart + ammVertsBuffsBef;
            while(b < bAfterEnd){
                ScnRenderVertexBuff_release(b);
                ++b;
            }
            ScnArraySorted_removeItemsAtIndex(&opq->vertexBuffs.arr, ammVertsBuffsBef, opq->vertexBuffs.arr.use - ammVertsBuffsBef);
        }
        //buffs
        if(ammBuffsBef < opq->buffs.arr.use){
            STScnRenderBuff* bStart = opq->buffs.arr.arr;
            const STScnRenderBuff* bAfterEnd = bStart + opq->buffs.arr.use;
            STScnRenderBuff* b = bStart + ammBuffsBef;
            while(b < bAfterEnd){
                ScnRenderBuff_release(b);
                ++b;
            }
            ScnArraySorted_removeItemsAtIndex(&opq->buffs.arr, ammBuffsBef, opq->buffs.arr.use - ammBuffsBef);
        }
    }
    return r;
}

//Buffers

ScnUI32 ScnRender_bufferCreate(STScnRenderRef ref, const STScnGpuBufferCfg* cfg){ //allocates a new buffer
    ScnUI32 r = 0;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(cfg != NULL){
        STScnRenderBuff b = STScnRenderBuff_Zero;
        b.buff = ScnGpuBuffer_alloc(opq->ctx);
        if(!ScnGpuBuffer_prepare(b.buff, cfg, NULL, NULL)){
            //error
            SCN_ASSERT(Scn_FALSE)
        } else {
            b.uid = ++opq->buffs.iSeq;
            ScnArraySorted_addPtr(opq->ctx, &opq->buffs.arr, &b, STScnRenderBuff);
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRender_bufferDestroy(STScnRenderRef ref, const ScnUI32 bid){ //flags a buffer for destruction
    ScnBOOL r = Scn_FALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnRenderBuff srch = STScnRenderBuff_Zero;
        srch.uid = bid;
        const ScnSI32 iFnd = ScnArraySorted_indexOf(&opq->buffs.arr, &srch);
        if(iFnd > 0){
            STScnRenderBuff* b = &opq->buffs.arr.arr[iFnd];
            ScnRenderBuff_release(b);
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//job
    
void ScnRender_jobStart(STScnRenderRef ref){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        opq->job.nodes.iDepth = 0;
        opq->job.nodes.ammOpen = 0;
        opq->job.nodes.ammPoppedAtEnd = 0;
        ScnArray_empty(&opq->job.nodes.arr);
        ScnArray_empty(&opq->job.cmds);
    }
    ScnMutex_unlock(opq->mutex);
}
    
ScnBOOL ScnRender_jobEnd(STScnRenderRef ref){
    ScnBOOL r = Scn_FALSE;
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
                    n->isPopped     = Scn_TRUE;
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
    ScnBOOL r = Scn_FALSE;
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
                n->isPopped     = Scn_TRUE;
                n->underCount   = (ScnUI32)(nAfterLast - n);
                n->cmds.size    = (ScnUI32)(opq->job.cmds.use - n->cmds.start);
                //
                --opq->job.nodes.ammOpen;
                r = Scn_TRUE;
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

//STScnRenderBuff

void ScnRenderBuff_release(STScnRenderBuff* obj){
    if(!ScnGpuBuffer_isNull(obj->buff)){
        ScnGpuBuffer_release(&obj->buff);
        ScnGpuBuffer_null(&obj->buff);
    }
}

//STScnRenderVertexBuff

void ScnRenderVertexBuff_release(STScnRenderVertexBuff* obj){
    if(!ScnGpuVertexbuff_isNull(obj->buff)){
        ScnGpuVertexbuff_release(&obj->buff);
        ScnGpuVertexbuff_null(&obj->buff);
    }
}
