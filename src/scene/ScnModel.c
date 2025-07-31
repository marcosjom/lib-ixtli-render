//
//  ScnModel.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 28/7/25.
//

#include "ixrender/scene/ScnModel.h"
#include "ixrender/core/ScnArray.h"
#include "ixrender/scene/ScnModelProps.h"

//STScnModelOpq

typedef struct STScnModelOpq_ {
    ScnContextRef       ctx;
    ScnMutexRef         mutex;
    //
    STScnModelProps     props;
    //cmds
    struct {
        ScnVertexbuffsRef vbuffs;
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
STScnModelDrawCmd* ScnModel_getDrawCmdsPtrLockedOpq_(STScnModelOpq* opq);
//

ScnSI32 ScnModel_getOpqSz(void){
    return (ScnSI32)sizeof(STScnModelOpq);
}

void ScnModel_initZeroedOpq(ScnContextRef ctx, void* obj) {
    STScnModelOpq* opq = (STScnModelOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex  = ScnContext_allocMutex(opq->ctx);
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
            ScnArray_foreach(&opq->cmds.heap, STScnModelDrawCmd, c,
               ScnModelDrawCmd_destroy(c);
            );
            ScnArray_destroy(opq->ctx, &opq->cmds.heap);
        }
        ScnVertexbuffs_releaseAndNull(&opq->cmds.vbuffs);
    }
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNull(&opq->ctx);
}

//

ScnUI32 ScnModel_getDrawCmdsCountLockedOpq_(STScnModelOpq* opq){
    return (opq->cmds.isHeap ? opq->cmds.heap.use : opq->cmds.embedded.isSet ? 1 : 0);
}

STScnModelDrawCmd* ScnModel_getDrawCmdsPtrLockedOpq_(STScnModelOpq* opq){
    return (opq->cmds.isHeap ? opq->cmds.heap.arr : opq->cmds.embedded.isSet ? &opq->cmds.embedded.cmd : NULL);
}

//

ScnBOOL ScnModel_setVertexBuffs(ScnModelRef ref, ScnVertexbuffsRef vbuffs){
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

STScnModelProps ScnModel_getProps(ScnModelRef ref){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->props;
}

//color

STScnColor8 ScnModel_getColor8(ScnModelRef ref){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->props.c8;
}

void ScnModel_setColor8(ScnModelRef ref, const STScnColor8 color){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.c8 = color;
}

void ScnModel_setColorRGBA8(ScnModelRef ref, const ScnUI8 r, const ScnUI8 g, const ScnUI8 b, const ScnUI8 a){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.c8.r = r;
    opq->props.c8.g = g;
    opq->props.c8.b = b;
    opq->props.c8.a = a;
}

//transform

STScnTransform ScnModel_getTransform(ScnModelRef ref){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->props.tform;
}

STScnPoint ScnModel_getTranslate(ScnModelRef ref){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (STScnPoint){ opq->props.tform.tx, opq->props.tform.ty };
}

STScnSize ScnModel_getScale(ScnModelRef ref){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (STScnSize){ opq->props.tform.sx, opq->props.tform.sy };
}

ScnFLOAT ScnModel_getRotDeg(ScnModelRef ref){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->props.tform.deg;
}

ScnFLOAT ScnModel_getRotRad(ScnModelRef ref){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return DEG_2_RAD(opq->props.tform.deg);
}

void ScnModel_setTranslate(ScnModelRef ref, const STScnPoint pos){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.tx = pos.x;
    opq->props.tform.ty = pos.y;
}

void ScnModel_setTranslateXY(ScnModelRef ref, const ScnFLOAT x, const ScnFLOAT y){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.tx = x;
    opq->props.tform.ty = y;
}

void ScnModel_setScale(ScnModelRef ref, const STScnSize s){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.sx = s.width;
    opq->props.tform.sy = s.height;
}

void ScnModel_setScaleWH(ScnModelRef ref, const ScnFLOAT sw, const ScnFLOAT sh){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.sx = sw;
    opq->props.tform.sy = sh;
}

void ScnModel_setRotDeg(ScnModelRef ref, const ScnFLOAT deg){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.deg = deg;
}

void ScnModel_setRotRad(ScnModelRef ref, const ScnFLOAT rad){
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.deg = RAD_2_DEG(rad);
}

//draw commands

void ScnModel_resetDrawCmds(ScnModelRef ref){
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
            ScnArray_foreach(&opq->cmds.heap, STScnModelDrawCmd, c,
               ScnModelDrawCmd_destroy(c);
            );
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

STScnVertexPtr ScnModel_addDraw(ScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 count){
    STScnVertexPtr r = STScnVertexPtr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && count > 0){
        STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v0Alloc(opq->cmds.vbuffs, count);
            if(r.ptr != NULL){
                STScnModelDrawCmd cmd;
                ScnModelDrawCmd_init(&cmd);
                ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
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

STScnVertexTexPtr ScnModel_addDrawTex(ScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnGpuTextureRef t0){
    STScnVertexTexPtr r = STScnVertexTexPtr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && count > 0){
        STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v1Alloc(opq->cmds.vbuffs, count);
            if(r.ptr != NULL){
                STScnModelDrawCmd cmd;
                ScnModelDrawCmd_init(&cmd);
                ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
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

STScnVertexTex2Ptr ScnModel_addDrawTex2(ScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnGpuTextureRef t0, ScnGpuTextureRef t1){
    STScnVertexTex2Ptr r = STScnVertexTex2Ptr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && count > 0){
        STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v2Alloc(opq->cmds.vbuffs, count);
            if(r.ptr != NULL){
                STScnModelDrawCmd cmd;
                ScnModelDrawCmd_init(&cmd);
                ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
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

STScnVertexTex3Ptr ScnModel_addDrawTex3(ScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnGpuTextureRef t0, ScnGpuTextureRef t1, ScnGpuTextureRef t2){
    STScnVertexTex3Ptr r = STScnVertexTex3Ptr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && count > 0){
        STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v3Alloc(opq->cmds.vbuffs, count);
            if(r.ptr != NULL){
                STScnModelDrawCmd cmd;
                ScnModelDrawCmd_init(&cmd);
                ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
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

STScnVertexIdxPtr ScnModel_addDrawIndexed(ScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, const ScnUI32 countVerts, STScnVertexPtr* dstVerts){
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
                    ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
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

STScnVertexIdxPtr ScnModel_addDrawIndexedTex(ScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnGpuTextureRef t0, const ScnUI32 countVerts, STScnVertexTexPtr* dstVerts){
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
                    ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
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

STScnVertexIdxPtr ScnModel_addDrawIndexedTex2(ScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnGpuTextureRef t0, ScnGpuTextureRef t1, const ScnUI32 countVerts, STScnVertexTex2Ptr* dstVerts){
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
                    ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
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

STScnVertexIdxPtr ScnModel_addDrawIndexedTex3(ScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnGpuTextureRef t0, ScnGpuTextureRef t1, ScnGpuTextureRef t2, const ScnUI32 countVerts, STScnVertexTex3Ptr* dstVerts){
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
                    ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
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

//draw commands to consumer

ScnBOOL ScnModel_sendRenderCmds(ScnModelRef ref, STScnModelPushItf* itf, void* itfParam){
    ScnBOOL r = ScnFALSE;
    STScnModelOpq* opq = (STScnModelOpq*)ScnSharedPtr_getOpq(ref.ptr);
    if(itf != NULL && itf->addCommandsWithProps != NULL){
        ScnMutex_lock(opq->mutex);
        {
            const STScnModelDrawCmd* cmds = ScnModel_getDrawCmdsPtrLockedOpq_(opq);
            const ScnUI32 cmdsSz = ScnModel_getDrawCmdsCountLockedOpq_(opq);
            //void* data, const STScnModelProps* props, const STScnModelDrawCmd* cmds, const ScnUI32 cmdsSz
            r = (*itf->addCommandsWithProps)(itfParam, &opq->props, cmds, cmdsSz);
        }
        ScnMutex_unlock(opq->mutex);
    }
    return r;
}

