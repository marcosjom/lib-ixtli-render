//
//  ScnModel.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 28/7/25.
//

#include "ixrender/scene/ScnModel.h"
#include "ixrender/core/ScnArray.h"
#include "ixrender/scene/ScnModelProps.h"

//ENScnModelDrawCmdType

typedef enum ENScnModelDrawCmdType_ {
    ENScnModelDrawCmdType_Undef = 0,
    //
    ENScnModelDrawCmdType_v0,
    ENScnModelDrawCmdType_v1,
    ENScnModelDrawCmdType_v2,
    ENScnModelDrawCmdType_v3,
    //
    ENScnModelDrawCmdType_i0,
    ENScnModelDrawCmdType_i1,
    ENScnModelDrawCmdType_i2,
    ENScnModelDrawCmdType_i3,
    //
    ENScnModelDrawCmdType_Count,
} ENScnModelDrawCmdType;

//STScnModelDrawCmd

typedef struct STScnModelDrawCmd_ {
    STScnRenderRef          rndr;   //reference to renderer, to keep vertices, indices and textures alive while the model exists.
    ENScnRenderShape        shape;
    ENScnModelDrawCmdType   type;
    //verts
    struct {
        STScnVertexPtr      v0;
        STScnVertexTexPtr   v1;
        STScnVertexTex2Ptr  v2;
        STScnVertexTex3Ptr  v3;
        ScnUI32             count;
    } verts;
    //idxs
    struct {
        STScnVertexIdxPtr   i0;
        STScnVertexIdxPtr   i1;
        STScnVertexIdxPtr   i2;
        STScnVertexIdxPtr   i3;
        ScnUI32             count;
    } idxs;
    //texs
    struct {
        STScnGpuTextureRef  t0;
        STScnGpuTextureRef  t1;
        STScnGpuTextureRef  t2;
    } texs;
} STScnModelDrawCmd;

void ScnModelDrawCmd_init(STScnModelDrawCmd* obj);
void ScnModelDrawCmd_destroy(STScnModelDrawCmd* obj);

//STScnModelOpq

typedef struct STScnModelOpq_ {
    STScnContextRef         ctx;
    STScnMutexRef           mutex;
    //
    STScnModelProps         props;
    //cmds
    struct {
        ScnBOOL             isHeap;
        union {
            //optimization, allow one command without allocating an array; most models will use one command.
            struct {
                ScnBOOL           isSet;
                STScnModelDrawCmd cmd;
            } embedded;
            ScnArrayStruct(heap, STScnModelDrawCmd);
        };
    } cmds;
} STScnModelOpq;

ScnSI32 ScnModel_getOpqSz(void){
    return (ScnSI32)sizeof(STScnModelOpq);
}

void ScnModel_initZeroedOpq(STScnContextRef ctx, void* obj) {
    STScnModelOpq* opq = (STScnModelOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex  = ScnContext_mutex_alloc(opq->ctx);
    //
    opq->props  = (STScnModelProps)STScnModelProps_Identity;
    //cmds
    {
        //
    }
}

void ScnModel_destroyOpq(void* obj){
    STScnModelOpq* opq = (STScnModelOpq*)obj;
    //cmds
    {
        if(!opq->cmds.isHeap){
            if(opq->cmds.embedded.isSet){
                ScnModelDrawCmd_destroy(&opq->cmds.embedded.cmd);
                opq->cmds.embedded.isSet = ScnFALSE;
            }
        } else {
            STScnModelDrawCmd* c = opq->cmds.heap.arr;
            STScnModelDrawCmd* cAfterEnd = c + opq->cmds.heap.use;
            while(c <  cAfterEnd){
                ScnModelDrawCmd_destroy(c);
                ++c;
            }
            ScnArray_destroy(opq->ctx, &opq->cmds.heap);
        }
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

//props

STScnModelProps ScnModel_getProps(STScnModelRef ref){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->props;
}

//color

STScnColor8 ScnModel_getColor8(STScnModelRef ref){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->props.c8;
}

void ScnModel_setColor8(STScnModelRef ref, const STScnColor8 color){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.c8 = color;
}

void ScnModel_setColorRGBA8(STScnModelRef ref, const ScnUI8 r, const ScnUI8 g, const ScnUI8 b, const ScnUI8 a){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.c8.r = r;
    opq->props.c8.g = g;
    opq->props.c8.b = b;
    opq->props.c8.a = a;
}

//transform

STScnTransform ScnModel_getTransform(STScnModelRef ref){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->props.tform;
}

STScnPoint ScnModel_getTranslate(STScnModelRef ref){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (STScnPoint){ opq->props.tform.tx, opq->props.tform.ty };
}

STScnSize ScnModel_getScale(STScnModelRef ref){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (STScnSize){ opq->props.tform.sx, opq->props.tform.sy };
}

ScnFLOAT ScnModel_getRotDeg(STScnModelRef ref){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->props.tform.deg;
}

ScnFLOAT ScnModel_getRotRad(STScnModelRef ref){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return DEG_2_RAD(opq->props.tform.deg);
}

void ScnModel_setTranslate(STScnModelRef ref, const STScnPoint pos){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.tx = pos.x;
    opq->props.tform.ty = pos.y;
}

void ScnModel_setTranslateXY(STScnModelRef ref, const ScnFLOAT x, const ScnFLOAT y){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.tx = x;
    opq->props.tform.ty = y;
}

void ScnModel_setScale(STScnModelRef ref, const STScnSize s){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.sx = s.width;
    opq->props.tform.sy = s.height;
}

void ScnModel_setScaleWH(STScnModelRef ref, const ScnFLOAT sw, const ScnFLOAT sh){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.sx = sw;
    opq->props.tform.sy = sh;
}

void ScnModel_setRotDeg(STScnModelRef ref, const ScnFLOAT deg){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.deg = deg;
}

void ScnModel_setRotRad(STScnModelRef ref, const ScnFLOAT rad){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.deg = RAD_2_DEG(rad);
}

//draw commands

void ScnModel_resetDrawCmds(STScnModelRef ref){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    //cmds
    {
        if(!opq->cmds.isHeap){
            if(opq->cmds.embedded.isSet){
                ScnModelDrawCmd_destroy(&opq->cmds.embedded.cmd);
                opq->cmds.embedded.isSet = ScnFALSE;
            }
        } else {
            STScnModelDrawCmd* c = opq->cmds.heap.arr;
            STScnModelDrawCmd* cAfterEnd = c + opq->cmds.heap.use;
            while(c <  cAfterEnd){
                ScnModelDrawCmd_destroy(c);
                ++c;
            }
            ScnArray_empty(&opq->cmds.heap);
        }
    }
    ScnMutex_unlock(opq->mutex);
}

ScnBOOL ScnModel_addDrawCmdLockedOpq_(STScnModelOpq* opq, STScnModelDrawCmd* cmd){
    ScnBOOL r = ScnFALSE;
    //analyze embedded
    if(!opq->cmds.isHeap){
        if(!opq->cmds.embedded.isSet){
            //set as embedded cmd
            opq->cmds.embedded.cmd = *cmd;
            opq->cmds.embedded.isSet = ScnTRUE;
            r = ScnTRUE;
        } else {
            //create heap array and move current embedded-itm to it
            STScnModelDrawCmd cpy = opq->cmds.embedded.cmd;
            opq->cmds.isHeap = ScnTRUE;
            ScnArray_init(opq->ctx, &opq->cmds.heap, 0, 8, STScnModelDrawCmd);
            if(NULL == ScnArray_addPtr(opq->ctx, &opq->cmds.heap, &cpy, STScnModelDrawCmd)){
                printf("ERROR, ScnArray_addPtr failed to add first heap array itm.\n");
                //revert
                ScnArray_destroy(opq->ctx, &opq->cmds.heap);
                opq->cmds.isHeap = ScnFALSE;
                opq->cmds.embedded.cmd =cpy;
            }
        }
    }
    //analyze heap
    if(opq->cmds.isHeap){
        if(NULL == ScnArray_addPtr(opq->ctx, &opq->cmds.heap, cmd, STScnModelDrawCmd)){
            printf("ERROR, ScnArray_addPtr failed to add heap array itm.\n");
        } else {
            r = ScnTRUE;
        }
    }
    return r;
}
    
ScnBOOL ScnModel_addDraw(STScnModelRef ref, const ENScnRenderShape shape, STScnVertexPtr verts, const ScnUI32 iFirst, const ScnUI32 count, STScnRenderRef rndr){
    ScnBOOL r = ScnFALSE;
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnModelDrawCmd cmd;
        ScnModelDrawCmd_init(&cmd);
        ScnRender_set(&cmd.rndr, rndr);
        cmd.type            = ENScnModelDrawCmdType_v0;
        cmd.shape           = shape;
        cmd.verts.v0.ptr    = verts.ptr + iFirst;
        cmd.verts.v0.idx    = verts.idx + iFirst;
        cmd.verts.count     = count;
        if(!ScnModel_addDrawCmdLockedOpq_(opq, &cmd)){
            printf("ERROR, ScnModel_addDrawCmdLockedOpq_ failed.\n");
            ScnModelDrawCmd_destroy(&cmd);
        } else {
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnModel_addDrawTex(STScnModelRef ref, const ENScnRenderShape shape, STScnVertexTexPtr verts, const ScnUI32 iFirst, const ScnUI32 count, STScnGpuTextureRef t0, STScnRenderRef rndr){
    ScnBOOL r = ScnFALSE;
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnModelDrawCmd cmd;
        ScnModelDrawCmd_init(&cmd);
        ScnRender_set(&cmd.rndr, rndr);
        cmd.type            = ENScnModelDrawCmdType_v1;
        cmd.shape           = shape;
        cmd.verts.v1.ptr    = verts.ptr + iFirst;
        cmd.verts.v1.idx    = verts.idx + iFirst;
        cmd.verts.count     = count;
        ScnGpuTexture_set(&cmd.texs.t0, t0);
        if(!ScnModel_addDrawCmdLockedOpq_(opq, &cmd)){
            printf("ERROR, ScnModel_addDrawCmdLockedOpq_ failed.\n");
            ScnModelDrawCmd_destroy(&cmd);
        } else {
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnModel_addDrawTex2(STScnModelRef ref, const ENScnRenderShape shape, STScnVertexTex2Ptr verts, const ScnUI32 iFirst, const ScnUI32 count, STScnGpuTextureRef t0, STScnGpuTextureRef t1, STScnRenderRef rndr){
    ScnBOOL r = ScnFALSE;
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnModelDrawCmd cmd;
        ScnModelDrawCmd_init(&cmd);
        ScnRender_set(&cmd.rndr, rndr);
        cmd.type            = ENScnModelDrawCmdType_v2;
        cmd.shape           = shape;
        cmd.verts.v2.ptr    = verts.ptr + iFirst;
        cmd.verts.v2.idx    = verts.idx + iFirst;
        cmd.verts.count     = count;
        ScnGpuTexture_set(&cmd.texs.t0, t0);
        ScnGpuTexture_set(&cmd.texs.t1, t1);
        if(!ScnModel_addDrawCmdLockedOpq_(opq, &cmd)){
            printf("ERROR, ScnModel_addDrawCmdLockedOpq_ failed.\n");
            ScnModelDrawCmd_destroy(&cmd);
        } else {
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnModel_addDrawTex3(STScnModelRef ref, const ENScnRenderShape shape, STScnVertexTex3Ptr verts, const ScnUI32 iFirst, const ScnUI32 count, STScnGpuTextureRef t0, STScnGpuTextureRef t1, STScnGpuTextureRef t2, STScnRenderRef rndr){
    ScnBOOL r = ScnFALSE;
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnModelDrawCmd cmd;
        ScnModelDrawCmd_init(&cmd);
        ScnRender_set(&cmd.rndr, rndr);
        cmd.type            = ENScnModelDrawCmdType_v3;
        cmd.shape           = shape;
        cmd.verts.v3.ptr    = verts.ptr + iFirst;
        cmd.verts.v3.idx    = verts.idx + iFirst;
        cmd.verts.count     = count;
        ScnGpuTexture_set(&cmd.texs.t0, t0);
        ScnGpuTexture_set(&cmd.texs.t1, t1);
        ScnGpuTexture_set(&cmd.texs.t2, t2);
        if(!ScnModel_addDrawCmdLockedOpq_(opq, &cmd)){
            printf("ERROR, ScnModel_addDrawCmdLockedOpq_ failed.\n");
            ScnModelDrawCmd_destroy(&cmd);
        } else {
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//

ScnBOOL ScnModel_addDrawIndexed(STScnModelRef ref, const ENScnRenderShape shape, STScnVertexIdxPtr idxs, const ScnUI32 iFirst, const ScnUI32 count, STScnRenderRef rndr){
    ScnBOOL r = ScnFALSE;
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnModelDrawCmd cmd;
        ScnModelDrawCmd_init(&cmd);
        ScnRender_set(&cmd.rndr, rndr);
        cmd.type            = ENScnModelDrawCmdType_i0;
        cmd.shape           = shape;
        cmd.idxs.i0.ptr     = idxs.ptr + iFirst;
        cmd.idxs.i0.idx     = idxs.idx + iFirst;
        cmd.idxs.count      = count;
        if(!ScnModel_addDrawCmdLockedOpq_(opq, &cmd)){
            printf("ERROR, ScnModel_addDrawIndexedAddLockedOpq_ failed.\n");
            ScnModelDrawCmd_destroy(&cmd);
        } else {
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnModel_addDrawIndexedTex(STScnModelRef ref, const ENScnRenderShape shape, STScnVertexIdxPtr idxs, const ScnUI32 iFirst, const ScnUI32 count, STScnGpuTextureRef t0, STScnRenderRef rndr){
    ScnBOOL r = ScnFALSE;
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnModelDrawCmd cmd;
        ScnModelDrawCmd_init(&cmd);
        ScnRender_set(&cmd.rndr, rndr);
        cmd.type            = ENScnModelDrawCmdType_i1;
        cmd.shape           = shape;
        cmd.idxs.i1.ptr     = idxs.ptr + iFirst;
        cmd.idxs.i1.idx     = idxs.idx + iFirst;
        cmd.idxs.count      = count;
        ScnGpuTexture_set(&cmd.texs.t0, t0);
        if(!ScnModel_addDrawCmdLockedOpq_(opq, &cmd)){
            printf("ERROR, ScnModel_addDrawIndexedAddLockedOpq_ failed.\n");
            ScnModelDrawCmd_destroy(&cmd);
        } else {
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnModel_addDrawIndexedTex2(STScnModelRef ref, const ENScnRenderShape shape, STScnVertexIdxPtr idxs, const ScnUI32 iFirst, const ScnUI32 count, STScnGpuTextureRef t0, STScnGpuTextureRef t1, STScnRenderRef rndr){
    ScnBOOL r = ScnFALSE;
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnModelDrawCmd cmd;
        ScnModelDrawCmd_init(&cmd);
        ScnRender_set(&cmd.rndr, rndr);
        cmd.type            = ENScnModelDrawCmdType_i2;
        cmd.shape           = shape;
        cmd.idxs.i2.ptr     = idxs.ptr + iFirst;
        cmd.idxs.i2.idx     = idxs.idx + iFirst;
        cmd.idxs.count      = count;
        ScnGpuTexture_set(&cmd.texs.t0, t0);
        ScnGpuTexture_set(&cmd.texs.t1, t1);
        if(!ScnModel_addDrawCmdLockedOpq_(opq, &cmd)){
            printf("ERROR, ScnModel_addDrawIndexedAddLockedOpq_ failed.\n");
            ScnModelDrawCmd_destroy(&cmd);
        } else {
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnModel_addDrawIndexedTex3(STScnModelRef ref, const ENScnRenderShape shape, STScnVertexIdxPtr idxs, const ScnUI32 iFirst, const ScnUI32 count, STScnGpuTextureRef t0, STScnGpuTextureRef t1, STScnGpuTextureRef t2, STScnRenderRef rndr){
    ScnBOOL r = ScnFALSE;
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnModelDrawCmd cmd;
        ScnModelDrawCmd_init(&cmd);
        ScnRender_set(&cmd.rndr, rndr);
        cmd.type            = ENScnModelDrawCmdType_i3;
        cmd.shape           = shape;
        cmd.idxs.i3.ptr     = idxs.ptr + iFirst;
        cmd.idxs.i3.idx     = idxs.idx + iFirst;
        cmd.idxs.count      = count;
        ScnGpuTexture_set(&cmd.texs.t0, t0);
        ScnGpuTexture_set(&cmd.texs.t1, t1);
        ScnGpuTexture_set(&cmd.texs.t2, t2);
        if(!ScnModel_addDrawCmdLockedOpq_(opq, &cmd)){
            printf("ERROR, ScnModel_addDrawIndexedAddLockedOpq_ failed.\n");
            ScnModelDrawCmd_destroy(&cmd);
        } else {
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//STScnModelDrawCmd

void ScnModelDrawCmd_init(STScnModelDrawCmd* obj){
    memset(obj, 0, sizeof(*obj));
}

void ScnModelDrawCmd_destroy(STScnModelDrawCmd* obj){
    //idxs
    {
        //
    }
    //verts
    {
        //
    }
    //texs
    {
        if(!ScnGpuTexture_isNull(obj->texs.t0)){
            ScnGpuTexture_release(&obj->texs.t0);
            ScnGpuTexture_null(&obj->texs.t0);
        }
        if(!ScnGpuTexture_isNull(obj->texs.t1)){
            ScnGpuTexture_release(&obj->texs.t1);
            ScnGpuTexture_null(&obj->texs.t1);
        }
        if(!ScnGpuTexture_isNull(obj->texs.t2)){
            ScnGpuTexture_release(&obj->texs.t2);
            ScnGpuTexture_null(&obj->texs.t2);
        }
    }
    //rndr
    if(!ScnRender_isNull(obj->rndr)){
        ScnRender_release(&obj->rndr);
        ScnRender_null(&obj->rndr);
    }
}
