//
//  ScnModel2D.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 28/7/25.
//

#include "ixrender/scene/ScnModel2D.h"
#include "ixrender/core/ScnArray.h"
#include "ixrender/scene/ScnModelProps2D.h"

//STScnModel2DOpq

typedef struct STScnModel2DOpq {
    ScnContextRef       ctx;
    ScnMutexRef         mutex;
    //
    STScnModelProps2D     props;
    //cmds
    struct {
        ScnVertexbuffsRef vbuffs;
        ScnBOOL             isHeap;
        union {
            //optimization, allow one command without allocating an array; most models will use one command.
            struct {
                ScnBOOL           isSet;
                STScnModel2DCmd cmd;
            } embedded;
            ScnArrayStruct(heap, STScnModel2DCmd);
        };
    } cmds;
} STScnModel2DOpq;

//

ScnUI32 ScnModel2D_getDrawCmdsCountLockedOpq_(STScnModel2DOpq* opq);
STScnModel2DCmd* ScnModel2D_getDrawCmdsPtrLockedOpq_(STScnModel2DOpq* opq);
//

ScnSI32 ScnModel2D_getOpqSz(void){
    return (ScnSI32)sizeof(STScnModel2DOpq);
}

void ScnModel2D_initZeroedOpq(ScnContextRef ctx, void* obj) {
    STScnModel2DOpq* opq = (STScnModel2DOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex  = ScnContext_allocMutex(opq->ctx);
    //
    opq->props  = (STScnModelProps2D)STScnModelProps2D_Identity;
    //cmds
    {
        //
    }
}

void ScnModel2D_destroyOpq(void* obj){
    STScnModel2DOpq* opq = (STScnModel2DOpq*)obj;
    //cmds
    {
        if(!opq->cmds.isHeap){
            if(opq->cmds.embedded.isSet){
                ScnModel2DCmd_destroy(&opq->cmds.embedded.cmd);
                opq->cmds.embedded.isSet = ScnFALSE;
            }
        } else {
            ScnArray_foreach(&opq->cmds.heap, STScnModel2DCmd, c,
               ScnModel2DCmd_destroy(c);
            );
            ScnArray_destroy(opq->ctx, &opq->cmds.heap);
        }
        ScnVertexbuffs_releaseAndNull(&opq->cmds.vbuffs);
    }
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNull(&opq->ctx);
}

//

ScnUI32 ScnModel2D_getDrawCmdsCountLockedOpq_(STScnModel2DOpq* opq){
    return (opq->cmds.isHeap ? opq->cmds.heap.use : opq->cmds.embedded.isSet ? 1 : 0);
}

STScnModel2DCmd* ScnModel2D_getDrawCmdsPtrLockedOpq_(STScnModel2DOpq* opq){
    return (opq->cmds.isHeap ? opq->cmds.heap.arr : opq->cmds.embedded.isSet ? &opq->cmds.embedded.cmd : NULL);
}

//

ScnBOOL ScnModel2D_setVertexBuffs(ScnModel2DRef ref, ScnVertexbuffsRef vbuffs){
    ScnBOOL r = ScnFALSE;
    STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(0 == ScnModel2D_getDrawCmdsCountLockedOpq_(opq)){
        ScnVertexbuffs_set(&opq->cmds.vbuffs, vbuffs);
        r = ScnTRUE;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//props

STScnModelProps2D ScnModel2D_getProps(ScnModel2DRef ref){
    STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->props;
}

//color

STScnColor8 ScnModel2D_getColor8(ScnModel2DRef ref){
    STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->props.c8;
}

void ScnModel2D_setColor8(ScnModel2DRef ref, const STScnColor8 color){
    STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.c8 = color;
}

void ScnModel2D_setColorRGBA8(ScnModel2DRef ref, const ScnUI8 r, const ScnUI8 g, const ScnUI8 b, const ScnUI8 a){
    STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.c8.r = r;
    opq->props.c8.g = g;
    opq->props.c8.b = b;
    opq->props.c8.a = a;
}

//transform

STScnTransform2D ScnModel2D_getTransform(ScnModel2DRef ref){
    STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->props.tform;
}

STScnPoint2D ScnModel2D_getTranslate(ScnModel2DRef ref){
    STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (STScnPoint2D){ opq->props.tform.tx, opq->props.tform.ty };
}

STScnSize2D ScnModel2D_getScale(ScnModel2DRef ref){
    STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (STScnSize2D){ opq->props.tform.sx, opq->props.tform.sy };
}

ScnFLOAT ScnModel2D_getRotDeg(ScnModel2DRef ref){
    STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->props.tform.deg;
}

ScnFLOAT ScnModel2D_getRotRad(ScnModel2DRef ref){
    STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return DEG_2_RAD(opq->props.tform.deg);
}

void ScnModel2D_setTranslate(ScnModel2DRef ref, const STScnPoint2D pos){
    STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.tx = pos.x;
    opq->props.tform.ty = pos.y;
}

void ScnModel2D_setTranslateXY(ScnModel2DRef ref, const ScnFLOAT x, const ScnFLOAT y){
    STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.tx = x;
    opq->props.tform.ty = y;
}

void ScnModel2D_setScale(ScnModel2DRef ref, const STScnSize2D s){
    STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.sx = s.width;
    opq->props.tform.sy = s.height;
}

void ScnModel2D_setScaleWH(ScnModel2DRef ref, const ScnFLOAT sw, const ScnFLOAT sh){
    STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.sx = sw;
    opq->props.tform.sy = sh;
}

void ScnModel2D_setRotDeg(ScnModel2DRef ref, const ScnFLOAT deg){
    STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.deg = deg;
}

void ScnModel2D_setRotRad(ScnModel2DRef ref, const ScnFLOAT rad){
    STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.deg = RAD_2_DEG(rad);
}

//draw commands

void ScnModel2D_resetDrawCmds(ScnModel2DRef ref){
    STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    //cmds
    {
        if(!opq->cmds.isHeap){
            if(opq->cmds.embedded.isSet){
                ScnModel2DCmd_destroy(&opq->cmds.embedded.cmd);
                opq->cmds.embedded.isSet = ScnFALSE;
            }
        } else {
            ScnArray_foreach(&opq->cmds.heap, STScnModel2DCmd, c,
               ScnModel2DCmd_destroy(c);
            );
            ScnArray_empty(&opq->cmds.heap);
        }
    }
    ScnMutex_unlock(opq->mutex);
}

ScnBOOL ScnModel2D_addDrawCmdLockedOpq_(STScnModel2DOpq* opq, STScnModel2DCmd* cmd){
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
            STScnModel2DCmd cpy = opq->cmds.embedded.cmd;
            opq->cmds.isHeap = ScnTRUE;
            ScnArray_init(opq->ctx, &opq->cmds.heap, 0, 8, STScnModel2DCmd);
            if(NULL == ScnArray_addPtr(opq->ctx, &opq->cmds.heap, &cpy, STScnModel2DCmd)){
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
        if(NULL == ScnArray_addPtr(opq->ctx, &opq->cmds.heap, cmd, STScnModel2DCmd)){
            printf("ERROR, ScnArray_addPtr failed to add heap array itm.\n");
        } else {
            r = ScnTRUE;
        }
    }
    return r;
}

STScnVertex2DPtr ScnModel2D_addDraw(ScnModel2DRef ref, const ENScnRenderShape shape, const ScnUI32 count){
    STScnVertex2DPtr r = STScnVertex2DPtr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && count > 0){
        STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v0Alloc(opq->cmds.vbuffs, count);
            if(r.ptr != NULL){
                STScnModel2DCmd cmd;
                ScnModel2DCmd_init(&cmd);
                ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
                cmd.shape       = shape;
                cmd.type        = ENScnModelDrawCmdType_2Dv0;
                cmd.verts.v0    = r;
                cmd.verts.count = count;
                if(!ScnModel2D_addDrawCmdLockedOpq_(opq, &cmd)){
                    printf("ERROR, ScnModel2D_addDrawCmdLockedOpq_ failed.\n");
                    ScnModel2DCmd_destroy(&cmd);
                    r = (STScnVertex2DPtr)STScnVertex2DPtr_Zero;
                }
            }
        }
        ScnMutex_unlock(opq->mutex);
    }
    return r;
}

STScnVertex2DTexPtr ScnModel2D_addDrawTex(ScnModel2DRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnGpuTextureRef t0){
    STScnVertex2DTexPtr r = STScnVertex2DTexPtr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && count > 0){
        STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v1Alloc(opq->cmds.vbuffs, count);
            if(r.ptr != NULL){
                STScnModel2DCmd cmd;
                ScnModel2DCmd_init(&cmd);
                ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
                cmd.shape       = shape;
                cmd.type        = ENScnModelDrawCmdType_2Dv1;
                cmd.verts.v1    = r;
                cmd.verts.count = count;
                if(!ScnModel2D_addDrawCmdLockedOpq_(opq, &cmd)){
                    printf("ERROR, ScnModel2D_addDrawCmdLockedOpq_ failed.\n");
                    ScnModel2DCmd_destroy(&cmd);
                    r = (STScnVertex2DTexPtr)STScnVertex2DTexPtr_Zero;
                }
            }
        }
        ScnMutex_unlock(opq->mutex);
    }
    return r;
}

STScnVertex2DTex2Ptr ScnModel2D_addDrawTex2(ScnModel2DRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnGpuTextureRef t0, ScnGpuTextureRef t1){
    STScnVertex2DTex2Ptr r = STScnVertex2DTex2Ptr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && count > 0){
        STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v2Alloc(opq->cmds.vbuffs, count);
            if(r.ptr != NULL){
                STScnModel2DCmd cmd;
                ScnModel2DCmd_init(&cmd);
                ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
                cmd.shape       = shape;
                cmd.type        = ENScnModelDrawCmdType_2Dv2;
                cmd.verts.v2    = r;
                cmd.verts.count = count;
                if(!ScnModel2D_addDrawCmdLockedOpq_(opq, &cmd)){
                    printf("ERROR, ScnModel2D_addDrawCmdLockedOpq_ failed.\n");
                    ScnModel2DCmd_destroy(&cmd);
                    r = (STScnVertex2DTex2Ptr)STScnVertex2DTex2Ptr_Zero;
                }
            }
        }
        ScnMutex_unlock(opq->mutex);
    }
    return r;
}

STScnVertex2DTex3Ptr ScnModel2D_addDrawTex3(ScnModel2DRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnGpuTextureRef t0, ScnGpuTextureRef t1, ScnGpuTextureRef t2){
    STScnVertex2DTex3Ptr r = STScnVertex2DTex3Ptr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && count > 0){
        STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v3Alloc(opq->cmds.vbuffs, count);
            if(r.ptr != NULL){
                STScnModel2DCmd cmd;
                ScnModel2DCmd_init(&cmd);
                ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
                cmd.shape       = shape;
                cmd.type        = ENScnModelDrawCmdType_2Dv3;
                cmd.verts.v3    = r;
                cmd.verts.count = count;
                if(!ScnModel2D_addDrawCmdLockedOpq_(opq, &cmd)){
                    printf("ERROR, ScnModel2D_addDrawCmdLockedOpq_ failed.\n");
                    ScnModel2DCmd_destroy(&cmd);
                    r = (STScnVertex2DTex3Ptr)STScnVertex2DTex3Ptr_Zero;
                }
            }
        }
        ScnMutex_unlock(opq->mutex);
    }
    return r;
}

//

STScnVertexIdxPtr ScnModel2D_addDrawIndexed(ScnModel2DRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, const ScnUI32 countVerts, STScnVertex2DPtr* dstVerts){
    STScnVertexIdxPtr r = STScnVertexIdxPtr_Zero;
    STScnVertex2DPtr v = STScnVertex2DPtr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && countIdxs > 0 && countVerts > 0){
        STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v0IdxsAlloc(opq->cmds.vbuffs, countIdxs);
            if(r.ptr != NULL){
                v = ScnVertexbuffs_v0Alloc(opq->cmds.vbuffs, countVerts);
                if(v.ptr == NULL){
                    ScnVertexbuffs_v0IdxsFree(opq->cmds.vbuffs, r);
                    r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                } else {
                    STScnModel2DCmd cmd;
                    ScnModel2DCmd_init(&cmd);
                    ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
                    cmd.shape       = shape;
                    cmd.type        = ENScnModelDrawCmdType_2Di0;
                    cmd.verts.v0    = v;
                    cmd.verts.count = countVerts;
                    cmd.idxs.i0     = r;
                    cmd.idxs.count  = countIdxs;
                    if(!ScnModel2D_addDrawCmdLockedOpq_(opq, &cmd)){
                        printf("ERROR, ScnModel2D_addDrawCmdLockedOpq_ failed.\n");
                        ScnModel2DCmd_destroy(&cmd);
                        r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                        v = (STScnVertex2DPtr)STScnVertex2DPtr_Zero;
                    }
                }
            }
        }
        ScnMutex_unlock(opq->mutex);
    }
    if(dstVerts != NULL) *dstVerts = v;
    return r;
}

STScnVertexIdxPtr ScnModel2D_addDrawIndexedTex(ScnModel2DRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnGpuTextureRef t0, const ScnUI32 countVerts, STScnVertex2DTexPtr* dstVerts){
    STScnVertexIdxPtr r = STScnVertexIdxPtr_Zero;
    STScnVertex2DTexPtr v = STScnVertex2DTexPtr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && countIdxs > 0 && countVerts > 0){
        STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v1IdxsAlloc(opq->cmds.vbuffs, countIdxs);
            if(r.ptr != NULL){
                v = ScnVertexbuffs_v1Alloc(opq->cmds.vbuffs, countVerts);
                if(v.ptr == NULL){
                    ScnVertexbuffs_v1IdxsFree(opq->cmds.vbuffs, r);
                    r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                } else {
                    STScnModel2DCmd cmd;
                    ScnModel2DCmd_init(&cmd);
                    ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
                    cmd.shape       = shape;
                    cmd.type        = ENScnModelDrawCmdType_2Di1;
                    cmd.verts.v1    = v;
                    cmd.verts.count = countVerts;
                    cmd.idxs.i1     = r;
                    cmd.idxs.count  = countIdxs;
                    if(!ScnModel2D_addDrawCmdLockedOpq_(opq, &cmd)){
                        printf("ERROR, ScnModel2D_addDrawCmdLockedOpq_ failed.\n");
                        ScnModel2DCmd_destroy(&cmd);
                        r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                        v = (STScnVertex2DTexPtr)STScnVertex2DTexPtr_Zero;
                    }
                }
            }
        }
        ScnMutex_unlock(opq->mutex);
    }
    if(dstVerts != NULL) *dstVerts = v;
    return r;
}

STScnVertexIdxPtr ScnModel2D_addDrawIndexedTex2(ScnModel2DRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnGpuTextureRef t0, ScnGpuTextureRef t1, const ScnUI32 countVerts, STScnVertex2DTex2Ptr* dstVerts){
    STScnVertexIdxPtr r = STScnVertexIdxPtr_Zero;
    STScnVertex2DTex2Ptr v = STScnVertex2DTex2Ptr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && countIdxs > 0 && countVerts > 0){
        STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v2IdxsAlloc(opq->cmds.vbuffs, countIdxs);
            if(r.ptr != NULL){
                v = ScnVertexbuffs_v2Alloc(opq->cmds.vbuffs, countVerts);
                if(v.ptr == NULL){
                    ScnVertexbuffs_v2IdxsFree(opq->cmds.vbuffs, r);
                    r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                } else {
                    STScnModel2DCmd cmd;
                    ScnModel2DCmd_init(&cmd);
                    ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
                    cmd.shape       = shape;
                    cmd.type        = ENScnModelDrawCmdType_2Di2;
                    cmd.verts.v2    = v;
                    cmd.verts.count = countVerts;
                    cmd.idxs.i2     = r;
                    cmd.idxs.count  = countIdxs;
                    if(!ScnModel2D_addDrawCmdLockedOpq_(opq, &cmd)){
                        printf("ERROR, ScnModel2D_addDrawCmdLockedOpq_ failed.\n");
                        ScnModel2DCmd_destroy(&cmd);
                        r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                        v = (STScnVertex2DTex2Ptr)STScnVertex2DTex2Ptr_Zero;
                    }
                }
            }
        }
        ScnMutex_unlock(opq->mutex);
    }
    if(dstVerts != NULL) *dstVerts = v;
    return r;
}

STScnVertexIdxPtr ScnModel2D_addDrawIndexedTex3(ScnModel2DRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnGpuTextureRef t0, ScnGpuTextureRef t1, ScnGpuTextureRef t2, const ScnUI32 countVerts, STScnVertex2DTex3Ptr* dstVerts){
    STScnVertexIdxPtr r = STScnVertexIdxPtr_Zero;
    STScnVertex2DTex3Ptr v = STScnVertex2DTex3Ptr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && countIdxs > 0 && countVerts > 0){
        STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v3IdxsAlloc(opq->cmds.vbuffs, countIdxs);
            if(r.ptr != NULL){
                v = ScnVertexbuffs_v3Alloc(opq->cmds.vbuffs, countVerts);
                if(v.ptr == NULL){
                    ScnVertexbuffs_v3IdxsFree(opq->cmds.vbuffs, r);
                    r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                } else {
                    STScnModel2DCmd cmd;
                    ScnModel2DCmd_init(&cmd);
                    ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
                    cmd.shape       = shape;
                    cmd.type        = ENScnModelDrawCmdType_2Di3;
                    cmd.verts.v3    = v;
                    cmd.verts.count = countVerts;
                    cmd.idxs.i3     = r;
                    cmd.idxs.count  = countIdxs;
                    if(!ScnModel2D_addDrawCmdLockedOpq_(opq, &cmd)){
                        printf("ERROR, ScnModel2D_addDrawCmdLockedOpq_ failed.\n");
                        ScnModel2DCmd_destroy(&cmd);
                        r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                        v = (STScnVertex2DTex3Ptr)STScnVertex2DTex3Ptr_Zero;
                    }
                }
            }
        }
        ScnMutex_unlock(opq->mutex);
    }
    if(dstVerts != NULL) *dstVerts = v;
    return r;
}

//draw commands to consumer

ScnBOOL ScnModel2D_sendRenderCmds(ScnModel2DRef ref, STScnModel2DPushItf* itf, void* itfParam){
    ScnBOOL r = ScnFALSE;
    STScnModel2DOpq* opq = (STScnModel2DOpq*)ScnSharedPtr_getOpq(ref.ptr);
    if(itf != NULL && itf->addCommandsWithProps != NULL){
        ScnMutex_lock(opq->mutex);
        {
            const STScnModel2DCmd* cmds = ScnModel2D_getDrawCmdsPtrLockedOpq_(opq);
            const ScnUI32 cmdsSz = ScnModel2D_getDrawCmdsCountLockedOpq_(opq);
            //void* data, const STScnModelProps2D* props, const STScnModel2DCmd* cmds, const ScnUI32 cmdsSz
            r = (*itf->addCommandsWithProps)(itfParam, &opq->props, cmds, cmdsSz);
        }
        ScnMutex_unlock(opq->mutex);
    }
    return r;
}

