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
    ENScnRenderShape        shape;
    ENScnModelDrawCmdType   type;
    STScnVertexbuffsRef     vbuffs;
    //verts
    struct {
        union {
            STScnVertexPtr      v0;
            STScnVertexTexPtr   v1;
            STScnVertexTex2Ptr  v2;
            STScnVertexTex3Ptr  v3;
        };
        ScnUI32             count;
    } verts;
    //idxs
    struct {
        union {
            STScnVertexIdxPtr   i0;
            STScnVertexIdxPtr   i1;
            STScnVertexIdxPtr   i2;
            STScnVertexIdxPtr   i3;
        };
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
        STScnVertexbuffsRef vbuffs;
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

//

ScnUI32 ScnModel_getDrawCmdsCountLockedOpq_(STScnModelOpq* opq);

//

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
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            ScnVertexbuffs_release(&opq->cmds.vbuffs);
            ScnVertexbuffs_null(&opq->cmds.vbuffs);
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

//

ScnUI32 ScnModel_getDrawCmdsCountLockedOpq_(STScnModelOpq* opq){
    return (opq->cmds.isHeap ? opq->cmds.heap.use : opq->cmds.embedded.isSet ? 1 : 0);
}

//

ScnBOOL ScnModel_setVertexBuffs(STScnModelRef ref, STScnVertexbuffsRef vbuffs){
    ScnBOOL r = ScnFALSE;
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(0 == ScnModel_getDrawCmdsCountLockedOpq_(opq)){
        ScnVertexbuffs_set(&opq->cmds.vbuffs, vbuffs);
        r = ScnTRUE;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
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

STScnVertexPtr ScnModel_addDraw(STScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 count){
    STScnVertexPtr r = STScnVertexPtr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && count > 0){
        STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v0Alloc(opq->cmds.vbuffs, count);
            if(r.ptr != NULL){
                STScnModelDrawCmd cmd;
                ScnModelDrawCmd_init(&cmd);
                ScnVertexbuffs_set(&opq->cmds.vbuffs, opq->cmds.vbuffs);
                cmd.shape       = shape;
                cmd.type        = ENScnModelDrawCmdType_v0;
                cmd.verts.v0    = r;
                cmd.verts.count = count;
                if(!ScnModel_addDrawCmdLockedOpq_(opq, &cmd)){
                    printf("ERROR, ScnModel_addDrawCmdLockedOpq_ failed.\n");
                    ScnModelDrawCmd_destroy(&cmd);
                    r = (STScnVertexPtr)STScnVertexPtr_Zero;
                }
            }
        }
        ScnMutex_unlock(opq->mutex);
    }
    return r;
}

STScnVertexTexPtr ScnModel_addDrawTex(STScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 count, STScnGpuTextureRef t0){
    STScnVertexTexPtr r = STScnVertexTexPtr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && count > 0){
        STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v1Alloc(opq->cmds.vbuffs, count);
            if(r.ptr != NULL){
                STScnModelDrawCmd cmd;
                ScnModelDrawCmd_init(&cmd);
                ScnVertexbuffs_set(&opq->cmds.vbuffs, opq->cmds.vbuffs);
                cmd.shape       = shape;
                cmd.type        = ENScnModelDrawCmdType_v1;
                cmd.verts.v1    = r;
                cmd.verts.count = count;
                if(!ScnModel_addDrawCmdLockedOpq_(opq, &cmd)){
                    printf("ERROR, ScnModel_addDrawCmdLockedOpq_ failed.\n");
                    ScnModelDrawCmd_destroy(&cmd);
                    r = (STScnVertexTexPtr)STScnVertexTexPtr_Zero;
                }
            }
        }
        ScnMutex_unlock(opq->mutex);
    }
    return r;
}

STScnVertexTex2Ptr ScnModel_addDrawTex2(STScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 count, STScnGpuTextureRef t0, STScnGpuTextureRef t1){
    STScnVertexTex2Ptr r = STScnVertexTex2Ptr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && count > 0){
        STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v2Alloc(opq->cmds.vbuffs, count);
            if(r.ptr != NULL){
                STScnModelDrawCmd cmd;
                ScnModelDrawCmd_init(&cmd);
                ScnVertexbuffs_set(&opq->cmds.vbuffs, opq->cmds.vbuffs);
                cmd.shape       = shape;
                cmd.type        = ENScnModelDrawCmdType_v2;
                cmd.verts.v2    = r;
                cmd.verts.count = count;
                if(!ScnModel_addDrawCmdLockedOpq_(opq, &cmd)){
                    printf("ERROR, ScnModel_addDrawCmdLockedOpq_ failed.\n");
                    ScnModelDrawCmd_destroy(&cmd);
                    r = (STScnVertexTex2Ptr)STScnVertexTex2Ptr_Zero;
                }
            }
        }
        ScnMutex_unlock(opq->mutex);
    }
    return r;
}

STScnVertexTex3Ptr ScnModel_addDrawTex3(STScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 count, STScnGpuTextureRef t0, STScnGpuTextureRef t1, STScnGpuTextureRef t2){
    STScnVertexTex3Ptr r = STScnVertexTex3Ptr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && count > 0){
        STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v3Alloc(opq->cmds.vbuffs, count);
            if(r.ptr != NULL){
                STScnModelDrawCmd cmd;
                ScnModelDrawCmd_init(&cmd);
                ScnVertexbuffs_set(&opq->cmds.vbuffs, opq->cmds.vbuffs);
                cmd.shape       = shape;
                cmd.type        = ENScnModelDrawCmdType_v3;
                cmd.verts.v3    = r;
                cmd.verts.count = count;
                if(!ScnModel_addDrawCmdLockedOpq_(opq, &cmd)){
                    printf("ERROR, ScnModel_addDrawCmdLockedOpq_ failed.\n");
                    ScnModelDrawCmd_destroy(&cmd);
                    r = (STScnVertexTex3Ptr)STScnVertexTex3Ptr_Zero;
                }
            }
        }
        ScnMutex_unlock(opq->mutex);
    }
    return r;
}

//

STScnVertexIdxPtr ScnModel_addDrawIndexed(STScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, const ScnUI32 countVerts, STScnVertexPtr* dstVerts){
    STScnVertexIdxPtr r = STScnVertexIdxPtr_Zero;
    STScnVertexPtr v = STScnVertexPtr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && countIdxs > 0 && countVerts > 0){
        STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v0IdxsAlloc(opq->cmds.vbuffs, countIdxs);
            if(r.ptr != NULL){
                v = ScnVertexbuffs_v0Alloc(opq->cmds.vbuffs, countVerts);
                if(v.ptr == NULL){
                    ScnVertexbuffs_v0IdxsFree(opq->cmds.vbuffs, r);
                    r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                } else {
                    STScnModelDrawCmd cmd;
                    ScnModelDrawCmd_init(&cmd);
                    ScnVertexbuffs_set(&opq->cmds.vbuffs, opq->cmds.vbuffs);
                    cmd.shape       = shape;
                    cmd.type        = ENScnModelDrawCmdType_i0;
                    cmd.verts.v0    = v;
                    cmd.verts.count = countVerts;
                    cmd.idxs.i0     = r;
                    cmd.idxs.count  = countIdxs;
                    if(!ScnModel_addDrawCmdLockedOpq_(opq, &cmd)){
                        printf("ERROR, ScnModel_addDrawCmdLockedOpq_ failed.\n");
                        ScnModelDrawCmd_destroy(&cmd);
                        r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                        v = (STScnVertexPtr)STScnVertexPtr_Zero;
                    }
                }
            }
        }
        ScnMutex_unlock(opq->mutex);
    }
    if(dstVerts != NULL) *dstVerts = v;
    return r;
}

STScnVertexIdxPtr ScnModel_addDrawIndexedTex(STScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, STScnGpuTextureRef t0, const ScnUI32 countVerts, STScnVertexTexPtr* dstVerts){
    STScnVertexIdxPtr r = STScnVertexIdxPtr_Zero;
    STScnVertexTexPtr v = STScnVertexTexPtr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && countIdxs > 0 && countVerts > 0){
        STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v1IdxsAlloc(opq->cmds.vbuffs, countIdxs);
            if(r.ptr != NULL){
                v = ScnVertexbuffs_v1Alloc(opq->cmds.vbuffs, countVerts);
                if(v.ptr == NULL){
                    ScnVertexbuffs_v1IdxsFree(opq->cmds.vbuffs, r);
                    r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                } else {
                    STScnModelDrawCmd cmd;
                    ScnModelDrawCmd_init(&cmd);
                    ScnVertexbuffs_set(&opq->cmds.vbuffs, opq->cmds.vbuffs);
                    cmd.shape       = shape;
                    cmd.type        = ENScnModelDrawCmdType_i1;
                    cmd.verts.v1    = v;
                    cmd.verts.count = countVerts;
                    cmd.idxs.i1     = r;
                    cmd.idxs.count  = countIdxs;
                    if(!ScnModel_addDrawCmdLockedOpq_(opq, &cmd)){
                        printf("ERROR, ScnModel_addDrawCmdLockedOpq_ failed.\n");
                        ScnModelDrawCmd_destroy(&cmd);
                        r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                        v = (STScnVertexTexPtr)STScnVertexTexPtr_Zero;
                    }
                }
            }
        }
        ScnMutex_unlock(opq->mutex);
    }
    if(dstVerts != NULL) *dstVerts = v;
    return r;
}

STScnVertexIdxPtr ScnModel_addDrawIndexedTex2(STScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, STScnGpuTextureRef t0, STScnGpuTextureRef t1, const ScnUI32 countVerts, STScnVertexTex2Ptr* dstVerts){
    STScnVertexIdxPtr r = STScnVertexIdxPtr_Zero;
    STScnVertexTex2Ptr v = STScnVertexTex2Ptr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && countIdxs > 0 && countVerts > 0){
        STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v2IdxsAlloc(opq->cmds.vbuffs, countIdxs);
            if(r.ptr != NULL){
                v = ScnVertexbuffs_v2Alloc(opq->cmds.vbuffs, countVerts);
                if(v.ptr == NULL){
                    ScnVertexbuffs_v2IdxsFree(opq->cmds.vbuffs, r);
                    r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                } else {
                    STScnModelDrawCmd cmd;
                    ScnModelDrawCmd_init(&cmd);
                    ScnVertexbuffs_set(&opq->cmds.vbuffs, opq->cmds.vbuffs);
                    cmd.shape       = shape;
                    cmd.type        = ENScnModelDrawCmdType_i2;
                    cmd.verts.v2    = v;
                    cmd.verts.count = countVerts;
                    cmd.idxs.i2     = r;
                    cmd.idxs.count  = countIdxs;
                    if(!ScnModel_addDrawCmdLockedOpq_(opq, &cmd)){
                        printf("ERROR, ScnModel_addDrawCmdLockedOpq_ failed.\n");
                        ScnModelDrawCmd_destroy(&cmd);
                        r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                        v = (STScnVertexTex2Ptr)STScnVertexTex2Ptr_Zero;
                    }
                }
            }
        }
        ScnMutex_unlock(opq->mutex);
    }
    if(dstVerts != NULL) *dstVerts = v;
    return r;
}

STScnVertexIdxPtr ScnModel_addDrawIndexedTex3(STScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, STScnGpuTextureRef t0, STScnGpuTextureRef t1, STScnGpuTextureRef t2, const ScnUI32 countVerts, STScnVertexTex3Ptr* dstVerts){
    STScnVertexIdxPtr r = STScnVertexIdxPtr_Zero;
    STScnVertexTex3Ptr v = STScnVertexTex3Ptr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && countIdxs > 0 && countVerts > 0){
        STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v3IdxsAlloc(opq->cmds.vbuffs, countIdxs);
            if(r.ptr != NULL){
                v = ScnVertexbuffs_v3Alloc(opq->cmds.vbuffs, countVerts);
                if(v.ptr == NULL){
                    ScnVertexbuffs_v3IdxsFree(opq->cmds.vbuffs, r);
                    r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                } else {
                    STScnModelDrawCmd cmd;
                    ScnModelDrawCmd_init(&cmd);
                    ScnVertexbuffs_set(&opq->cmds.vbuffs, opq->cmds.vbuffs);
                    cmd.shape       = shape;
                    cmd.type        = ENScnModelDrawCmdType_i3;
                    cmd.verts.v3    = v;
                    cmd.verts.count = countVerts;
                    cmd.idxs.i3     = r;
                    cmd.idxs.count  = countIdxs;
                    if(!ScnModel_addDrawCmdLockedOpq_(opq, &cmd)){
                        printf("ERROR, ScnModel_addDrawCmdLockedOpq_ failed.\n");
                        ScnModelDrawCmd_destroy(&cmd);
                        r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                        v = (STScnVertexTex3Ptr)STScnVertexTex3Ptr_Zero;
                    }
                }
            }
        }
        ScnMutex_unlock(opq->mutex);
    }
    if(dstVerts != NULL) *dstVerts = v;
    return r;
}

//STScnModelDrawCmd

void ScnModelDrawCmd_init(STScnModelDrawCmd* obj){
    memset(obj, 0, sizeof(*obj));
}

void ScnModelDrawCmd_destroy(STScnModelDrawCmd* obj){
    //idxs
    //verts
    SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs) || (obj->verts.count == 0 && obj->verts.v0.ptr == NULL && obj->verts.v0.idx == 0 && obj->idxs.count == 0 && obj->idxs.i0.ptr == NULL && obj->idxs.i0.idx == 0))
    switch(obj->type){
        case ENScnModelDrawCmdType_v0:
            SCN_ASSERT(obj->idxs.count == 0 && obj->idxs.i0.ptr == NULL && obj->idxs.i0.idx == 0)
            if(obj->verts.v0.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v0Free(obj->vbuffs, obj->verts.v0);
                }
                obj->verts.v0 = (STScnVertexPtr)STScnVertexPtr_Zero;
            }
            break;
        case ENScnModelDrawCmdType_v1:
            SCN_ASSERT(obj->idxs.count == 0 && obj->idxs.i1.ptr == NULL && obj->idxs.i1.idx == 0)
            if(obj->verts.v1.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v1Free(obj->vbuffs, obj->verts.v1);
                }
                obj->verts.v1 = (STScnVertexTexPtr)STScnVertexTexPtr_Zero;
            }
            break;
        case ENScnModelDrawCmdType_v2:
            SCN_ASSERT(obj->idxs.count == 0 && obj->idxs.i2.ptr == NULL && obj->idxs.i2.idx == 0)
            if(obj->verts.v2.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v2Free(obj->vbuffs, obj->verts.v2);
                }
                obj->verts.v2 = (STScnVertexTex2Ptr)STScnVertexTex2Ptr_Zero;
            }
            break;
        case ENScnModelDrawCmdType_v3:
            SCN_ASSERT(obj->idxs.count == 0 && obj->idxs.i3.ptr == NULL && obj->idxs.i3.idx == 0)
            if(obj->verts.v3.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v3Free(obj->vbuffs, obj->verts.v3);
                }
                obj->verts.v3 = (STScnVertexTex3Ptr)STScnVertexTex3Ptr_Zero;
            }
            break;
            //
        case ENScnModelDrawCmdType_i0:
            if(obj->verts.v0.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v0Free(obj->vbuffs, obj->verts.v0);
                }
                obj->verts.v0 = (STScnVertexPtr)STScnVertexPtr_Zero;
            }
            if(obj->idxs.i0.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v0IdxsFree(obj->vbuffs, obj->idxs.i0);
                }
                obj->idxs.i0 = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
            }
            break;
        case ENScnModelDrawCmdType_i1:
            if(obj->verts.v1.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v1Free(obj->vbuffs, obj->verts.v1);
                }
                obj->verts.v1 = (STScnVertexTexPtr)STScnVertexTexPtr_Zero;
            }
            if(obj->idxs.i1.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v1IdxsFree(obj->vbuffs, obj->idxs.i1);
                }
                obj->idxs.i1 = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
            }
            break;
        case ENScnModelDrawCmdType_i2:
            if(obj->verts.v2.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v2Free(obj->vbuffs, obj->verts.v2);
                }
                obj->verts.v2 = (STScnVertexTex2Ptr)STScnVertexTex2Ptr_Zero;
            }
            if(obj->idxs.i2.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v2IdxsFree(obj->vbuffs, obj->idxs.i2);
                }
                obj->idxs.i2 = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
            }
            break;
        case ENScnModelDrawCmdType_i3:
            if(obj->verts.v3.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v3Free(obj->vbuffs, obj->verts.v3);
                }
                obj->verts.v3 = (STScnVertexTex3Ptr)STScnVertexTex3Ptr_Zero;
            }
            if(obj->idxs.i3.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v3IdxsFree(obj->vbuffs, obj->idxs.i3);
                }
                obj->idxs.i3 = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
            }
            break;
        default:
            SCN_ASSERT(obj->verts.count == 0 && obj->verts.v0.ptr == NULL && obj->verts.v0.idx == 0 && obj->idxs.count == 0 && obj->idxs.i0.ptr == NULL && obj->idxs.i0.idx == 0)
            break;
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
    //vbuffs
    if(!ScnVertexbuffs_isNull(obj->vbuffs)){
        ScnVertexbuffs_release(&obj->vbuffs);
        ScnVertexbuffs_null(&obj->vbuffs);
    }
}
