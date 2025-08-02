//
//  ScnModel2d.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 28/7/25.
//

#include "ixrender/scene/ScnModel2d.h"
#include "ixrender/core/ScnArrayEmbed.h"

//STScnModel2dOpq

typedef struct STScnModel2dOpq {
    ScnContextRef       ctx;
    ScnMutexRef         mutex;
    ScnVertexbuffsRef   vbuffs;
    ScnArrayEmbedStruct(cmds, STScnModel2dCmd);
} STScnModel2dOpq;

//

ScnSI32 ScnModel2d_getOpqSz(void){
    return (ScnSI32)sizeof(STScnModel2dOpq);
}

void ScnModel2d_initZeroedOpq(ScnContextRef ctx, void* obj) {
    STScnModel2dOpq* opq = (STScnModel2dOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex  = ScnContext_allocMutex(opq->ctx);
    //cmds
    {
        ScnArrayEmbed_init(opq->ctx, &opq->cmds, 0, 16, STScnModel2dCmd);
    }
}

void ScnModel2d_destroyOpq(void* obj){
    STScnModel2dOpq* opq = (STScnModel2dOpq*)obj;
    //cmds
    {
        ScnArrayEmbed_foreach(&opq->cmds, STScnModel2dCmd, cmd,
                              ScnModel2dCmd_destroy(cmd);
                              );
        ScnArrayEmbed_destroy(opq->ctx, &opq->cmds);
    }
    ScnVertexbuffs_releaseAndNull(&opq->vbuffs);
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNull(&opq->ctx);
}

//

ScnBOOL ScnModel2d_setVertexBuffs(ScnModel2dRef ref, ScnVertexbuffsRef vbuffs){
    ScnBOOL r = ScnFALSE;
    STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(0 == ScnArrayEmbed_getUse(&opq->cmds)){
        ScnVertexbuffs_set(&opq->vbuffs, vbuffs);
        r = ScnTRUE;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//draw commands

void ScnModel2d_resetDrawCmds(ScnModel2dRef ref){
    STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    //cmds
    {
        ScnArrayEmbed_foreach(&opq->cmds, STScnModel2dCmd, cmd,
                              ScnModel2dCmd_destroy(cmd);
                              );
        ScnArrayEmbed_empty(&opq->cmds);
    }
    ScnMutex_unlock(opq->mutex);
}


STScnVertex2DPtr ScnModel2d_addDraw(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 count){
    STScnVertex2DPtr r = STScnVertex2DPtr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && count > 0){
        STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->vbuffs)){
            r = ScnVertexbuffs_v0Alloc(opq->vbuffs, count);
            if(r.ptr != NULL){
                STScnModel2dCmd* itm = NULL;
                STScnModel2dCmd cmd;
                ScnModel2dCmd_init(&cmd);
                ScnVertexbuffs_set(&cmd.vbuffs, opq->vbuffs);
                cmd.shape       = shape;
                cmd.type        = ENScnModelDrawCmdType_2Dv0;
                cmd.verts.v0    = r;
                cmd.verts.count = count;
                ScnArrayEmbed_addPtr(itm, opq->ctx, &opq->cmds, &cmd, STScnModel2dCmd);
                if(itm == NULL){
                    printf("ERROR, ScnModel2d_addDraw::ScnArrayEmbed_addPtr failed.\n");
                    ScnModel2dCmd_destroy(&cmd);
                    r = (STScnVertex2DPtr)STScnVertex2DPtr_Zero;
                }
            }
        }
        ScnMutex_unlock(opq->mutex);
    }
    return r;
}

STScnVertex2DTexPtr ScnModel2d_addDrawTex(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnGpuTextureRef t0){
    STScnVertex2DTexPtr r = STScnVertex2DTexPtr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && count > 0){
        STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->vbuffs)){
            r = ScnVertexbuffs_v1Alloc(opq->vbuffs, count);
            if(r.ptr != NULL){
                STScnModel2dCmd* itm = NULL;
                STScnModel2dCmd cmd;
                ScnModel2dCmd_init(&cmd);
                ScnVertexbuffs_set(&cmd.vbuffs, opq->vbuffs);
                cmd.shape       = shape;
                cmd.type        = ENScnModelDrawCmdType_2Dv1;
                cmd.verts.v1    = r;
                cmd.verts.count = count;
                ScnArrayEmbed_addPtr(itm, opq->ctx, &opq->cmds, &cmd, STScnModel2dCmd);
                if(itm == NULL){
                    printf("ERROR, ScnModel2d_addDrawTex::ScnArrayEmbed_addPtr failed.\n");
                    ScnModel2dCmd_destroy(&cmd);
                    r = (STScnVertex2DTexPtr)STScnVertex2DTexPtr_Zero;
                }
            }
        }
        ScnMutex_unlock(opq->mutex);
    }
    return r;
}

STScnVertex2DTex2Ptr ScnModel2d_addDrawTex2(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnGpuTextureRef t0, ScnGpuTextureRef t1){
    STScnVertex2DTex2Ptr r = STScnVertex2DTex2Ptr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && count > 0){
        STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->vbuffs)){
            r = ScnVertexbuffs_v2Alloc(opq->vbuffs, count);
            if(r.ptr != NULL){
                STScnModel2dCmd* itm = NULL;
                STScnModel2dCmd cmd;
                ScnModel2dCmd_init(&cmd);
                ScnVertexbuffs_set(&cmd.vbuffs, opq->vbuffs);
                cmd.shape       = shape;
                cmd.type        = ENScnModelDrawCmdType_2Dv2;
                cmd.verts.v2    = r;
                cmd.verts.count = count;
                ScnArrayEmbed_addPtr(itm, opq->ctx, &opq->cmds, &cmd, STScnModel2dCmd);
                if(itm == NULL){
                    printf("ERROR, ScnModel2d_addDrawTex2::ScnArrayEmbed_addPtr failed.\n");
                    ScnModel2dCmd_destroy(&cmd);
                    r = (STScnVertex2DTex2Ptr)STScnVertex2DTex2Ptr_Zero;
                }
            }
        }
        ScnMutex_unlock(opq->mutex);
    }
    return r;
}

STScnVertex2DTex3Ptr ScnModel2d_addDrawTex3(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnGpuTextureRef t0, ScnGpuTextureRef t1, ScnGpuTextureRef t2){
    STScnVertex2DTex3Ptr r = STScnVertex2DTex3Ptr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && count > 0){
        STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->vbuffs)){
            r = ScnVertexbuffs_v3Alloc(opq->vbuffs, count);
            if(r.ptr != NULL){
                STScnModel2dCmd* itm = NULL;
                STScnModel2dCmd cmd;
                ScnModel2dCmd_init(&cmd);
                ScnVertexbuffs_set(&cmd.vbuffs, opq->vbuffs);
                cmd.shape       = shape;
                cmd.type        = ENScnModelDrawCmdType_2Dv3;
                cmd.verts.v3    = r;
                cmd.verts.count = count;
                ScnArrayEmbed_addPtr(itm, opq->ctx, &opq->cmds, &cmd, STScnModel2dCmd);
                if(itm == NULL){
                    printf("ERROR, ScnModel2d_addDrawTex3::ScnArrayEmbed_addPtr failed.\n");
                    ScnModel2dCmd_destroy(&cmd);
                    r = (STScnVertex2DTex3Ptr)STScnVertex2DTex3Ptr_Zero;
                }
            }
        }
        ScnMutex_unlock(opq->mutex);
    }
    return r;
}

//dbg
STScnModel2dCmd* ScnModel2d_findDrawVertCmdLockedOpq_(STScnModel2dOpq* opq, const ENScnModelDrawCmdType type, void* ptrPtr, const ScnUI32 sizePerItm, const ScnUI32 count){
    ScnArrayEmbed_foreach(&opq->cmds, STScnModel2dCmd, cmd,
        if(cmd->type == type && (void*)cmd->verts.v0.ptr <= (void*)ptrPtr && ((ScnBYTE*)ptrPtr + (sizePerItm * count)) <= ((ScnBYTE*)cmd->verts.v0.ptr + (sizePerItm * cmd->verts.count))){
            return cmd;
        }
    );
    return NULL;
}

//Call these if you updated the vertices values after last render pass.

ScnBOOL ScnModel2d_v0FlagForSync(ScnModel2dRef ref, STScnVertex2DPtr ptr, const ScnUI32 count){
    ScnBOOL r = ScnFALSE;
    STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnVertexbuffs_isNull(opq->vbuffs)){
        SCN_ASSERT(NULL != ScnModel2d_findDrawVertCmdLockedOpq_(opq, ENScnModelDrawCmdType_2Dv0, ptr.ptr, sizeof(ptr.ptr[0]), count)) //user logic error
        r = ScnVertexbuffs_v0Invalidate(opq->vbuffs, ptr, count);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnModel2d_v1FlagForSync(ScnModel2dRef ref, STScnVertex2DTexPtr ptr, const ScnUI32 count){
    ScnBOOL r = ScnFALSE;
    STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnVertexbuffs_isNull(opq->vbuffs)){
        SCN_ASSERT(NULL != ScnModel2d_findDrawVertCmdLockedOpq_(opq, ENScnModelDrawCmdType_2Dv1, ptr.ptr, sizeof(ptr.ptr[0]), count)) //user logic error
        r = ScnVertexbuffs_v1Invalidate(opq->vbuffs, ptr, count);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnModel2d_v2FlagForSync(ScnModel2dRef ref, STScnVertex2DTex2Ptr ptr, const ScnUI32 count){
    ScnBOOL r = ScnFALSE;
    STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnVertexbuffs_isNull(opq->vbuffs)){
        SCN_ASSERT(NULL != ScnModel2d_findDrawVertCmdLockedOpq_(opq, ENScnModelDrawCmdType_2Dv2, ptr.ptr, sizeof(ptr.ptr[0]), count)) //user logic error
        r = ScnVertexbuffs_v2Invalidate(opq->vbuffs, ptr, count);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnModel2d_v3FlagForSync(ScnModel2dRef ref, STScnVertex2DTex3Ptr ptr, const ScnUI32 count){
    ScnBOOL r = ScnFALSE;
    STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnVertexbuffs_isNull(opq->vbuffs)){
        SCN_ASSERT(NULL != ScnModel2d_findDrawVertCmdLockedOpq_(opq, ENScnModelDrawCmdType_2Dv3, ptr.ptr, sizeof(ptr.ptr[0]), count)) //user logic error
        r = ScnVertexbuffs_v3Invalidate(opq->vbuffs, ptr, count);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//

STScnVertexIdxPtr ScnModel2d_addDrawIndexed(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, const ScnUI32 countVerts, STScnVertex2DPtr* dstVerts){
    STScnVertexIdxPtr r = STScnVertexIdxPtr_Zero;
    STScnVertex2DPtr v = STScnVertex2DPtr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && countIdxs > 0 && countVerts > 0){
        STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->vbuffs)){
            r = ScnVertexbuffs_v0IdxsAlloc(opq->vbuffs, countIdxs);
            if(r.ptr != NULL){
                v = ScnVertexbuffs_v0Alloc(opq->vbuffs, countVerts);
                if(v.ptr == NULL){
                    ScnVertexbuffs_v0IdxsFree(opq->vbuffs, r);
                    r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                } else {
                    STScnModel2dCmd* itm = NULL;
                    STScnModel2dCmd cmd;
                    ScnModel2dCmd_init(&cmd);
                    ScnVertexbuffs_set(&cmd.vbuffs, opq->vbuffs);
                    cmd.shape       = shape;
                    cmd.type        = ENScnModelDrawCmdType_2Di0;
                    cmd.verts.v0    = v;
                    cmd.verts.count = countVerts;
                    cmd.idxs.i0     = r;
                    cmd.idxs.count  = countIdxs;
                    ScnArrayEmbed_addPtr(itm, opq->ctx, &opq->cmds, &cmd, STScnModel2dCmd);
                    if(itm == NULL){
                        printf("ERROR, ScnModel2d_addDrawIndexed::ScnArrayEmbed_addPtr failed.\n");
                        ScnModel2dCmd_destroy(&cmd);
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

STScnVertexIdxPtr ScnModel2d_addDrawIndexedTex(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnGpuTextureRef t0, const ScnUI32 countVerts, STScnVertex2DTexPtr* dstVerts){
    STScnVertexIdxPtr r = STScnVertexIdxPtr_Zero;
    STScnVertex2DTexPtr v = STScnVertex2DTexPtr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && countIdxs > 0 && countVerts > 0){
        STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->vbuffs)){
            r = ScnVertexbuffs_v1IdxsAlloc(opq->vbuffs, countIdxs);
            if(r.ptr != NULL){
                v = ScnVertexbuffs_v1Alloc(opq->vbuffs, countVerts);
                if(v.ptr == NULL){
                    ScnVertexbuffs_v1IdxsFree(opq->vbuffs, r);
                    r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                } else {
                    STScnModel2dCmd* itm = NULL;
                    STScnModel2dCmd cmd;
                    ScnModel2dCmd_init(&cmd);
                    ScnVertexbuffs_set(&cmd.vbuffs, opq->vbuffs);
                    cmd.shape       = shape;
                    cmd.type        = ENScnModelDrawCmdType_2Di1;
                    cmd.verts.v1    = v;
                    cmd.verts.count = countVerts;
                    cmd.idxs.i1     = r;
                    cmd.idxs.count  = countIdxs;
                    ScnArrayEmbed_addPtr(itm, opq->ctx, &opq->cmds, &cmd, STScnModel2dCmd);
                    if(itm == NULL){
                        printf("ERROR, ScnModel2d_addDrawIndexedTex::ScnArrayEmbed_addPtr failed.\n");
                        ScnModel2dCmd_destroy(&cmd);
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

STScnVertexIdxPtr ScnModel2d_addDrawIndexedTex2(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnGpuTextureRef t0, ScnGpuTextureRef t1, const ScnUI32 countVerts, STScnVertex2DTex2Ptr* dstVerts){
    STScnVertexIdxPtr r = STScnVertexIdxPtr_Zero;
    STScnVertex2DTex2Ptr v = STScnVertex2DTex2Ptr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && countIdxs > 0 && countVerts > 0){
        STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->vbuffs)){
            r = ScnVertexbuffs_v2IdxsAlloc(opq->vbuffs, countIdxs);
            if(r.ptr != NULL){
                v = ScnVertexbuffs_v2Alloc(opq->vbuffs, countVerts);
                if(v.ptr == NULL){
                    ScnVertexbuffs_v2IdxsFree(opq->vbuffs, r);
                    r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                } else {
                    STScnModel2dCmd* itm = NULL;
                    STScnModel2dCmd cmd;
                    ScnModel2dCmd_init(&cmd);
                    ScnVertexbuffs_set(&cmd.vbuffs, opq->vbuffs);
                    cmd.shape       = shape;
                    cmd.type        = ENScnModelDrawCmdType_2Di2;
                    cmd.verts.v2    = v;
                    cmd.verts.count = countVerts;
                    cmd.idxs.i2     = r;
                    cmd.idxs.count  = countIdxs;
                    ScnArrayEmbed_addPtr(itm, opq->ctx, &opq->cmds, &cmd, STScnModel2dCmd);
                    if(itm == NULL){
                        printf("ERROR, ScnModel2d_addDrawIndexedTex2::ScnArrayEmbed_addPtr failed.\n");
                        ScnModel2dCmd_destroy(&cmd);
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

STScnVertexIdxPtr ScnModel2d_addDrawIndexedTex3(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnGpuTextureRef t0, ScnGpuTextureRef t1, ScnGpuTextureRef t2, const ScnUI32 countVerts, STScnVertex2DTex3Ptr* dstVerts){
    STScnVertexIdxPtr r = STScnVertexIdxPtr_Zero;
    STScnVertex2DTex3Ptr v = STScnVertex2DTex3Ptr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && countIdxs > 0 && countVerts > 0){
        STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->vbuffs)){
            r = ScnVertexbuffs_v3IdxsAlloc(opq->vbuffs, countIdxs);
            if(r.ptr != NULL){
                v = ScnVertexbuffs_v3Alloc(opq->vbuffs, countVerts);
                if(v.ptr == NULL){
                    ScnVertexbuffs_v3IdxsFree(opq->vbuffs, r);
                    r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                } else {
                    STScnModel2dCmd* itm = NULL;
                    STScnModel2dCmd cmd;
                    ScnModel2dCmd_init(&cmd);
                    ScnVertexbuffs_set(&cmd.vbuffs, opq->vbuffs);
                    cmd.shape       = shape;
                    cmd.type        = ENScnModelDrawCmdType_2Di3;
                    cmd.verts.v3    = v;
                    cmd.verts.count = countVerts;
                    cmd.idxs.i3     = r;
                    cmd.idxs.count  = countIdxs;
                    ScnArrayEmbed_addPtr(itm, opq->ctx, &opq->cmds, &cmd, STScnModel2dCmd);
                    if(itm == NULL){
                        printf("ERROR, ScnModel2d_addDrawIndexedTex3::ScnArrayEmbed_addPtr failed.\n");
                        ScnModel2dCmd_destroy(&cmd);
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

//dbg
STScnModel2dCmd* ScnModel2d_findDrawIdxCmdLockedOpq_(STScnModel2dOpq* opq, const ENScnModelDrawCmdType type, STScnVertexIdxPtr ptr, const ScnUI32 count){
    ScnArrayEmbed_foreach(&opq->cmds, STScnModel2dCmd, cmd,
        if(cmd->type == type && cmd->idxs.i0.ptr <= ptr.ptr && (ptr.ptr + count) <= (cmd->idxs.i0.ptr + cmd->idxs.count)){
            return cmd;
        }
    );
    return NULL;
}

//Call these if you updated the vertices values after last render pass.

ScnBOOL ScnModel2d_i0FlagForSync(ScnModel2dRef ref, STScnVertexIdxPtr ptr, const ScnUI32 count){
    ScnBOOL r = ScnFALSE;
    STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnVertexbuffs_isNull(opq->vbuffs)){
        SCN_ASSERT(NULL != ScnModel2d_findDrawIdxCmdLockedOpq_(opq, ENScnModelDrawCmdType_2Di0, ptr, count)); //user logic error
        r = ScnVertexbuffs_v0IdxsInvalidate(opq->vbuffs, ptr, count);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnModel2d_i1FlagForSync(ScnModel2dRef ref, STScnVertexIdxPtr ptr, const ScnUI32 count){
    ScnBOOL r = ScnFALSE;
    STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnVertexbuffs_isNull(opq->vbuffs)){
        SCN_ASSERT(NULL != ScnModel2d_findDrawIdxCmdLockedOpq_(opq, ENScnModelDrawCmdType_2Di1, ptr, count)); //user logic error
        r = ScnVertexbuffs_v1IdxsInvalidate(opq->vbuffs, ptr, count);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnModel2d_i2FlagForSync(ScnModel2dRef ref, STScnVertexIdxPtr ptr, const ScnUI32 count){
    ScnBOOL r = ScnFALSE;
    STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnVertexbuffs_isNull(opq->vbuffs)){
        SCN_ASSERT(NULL != ScnModel2d_findDrawIdxCmdLockedOpq_(opq, ENScnModelDrawCmdType_2Di2, ptr, count)); //user logic error
        r = ScnVertexbuffs_v2IdxsInvalidate(opq->vbuffs, ptr, count);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnModel2d_i3FlagForSync(ScnModel2dRef ref, STScnVertexIdxPtr ptr, const ScnUI32 count){
    ScnBOOL r = ScnFALSE;
    STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnVertexbuffs_isNull(opq->vbuffs)){
        SCN_ASSERT(NULL != ScnModel2d_findDrawIdxCmdLockedOpq_(opq, ENScnModelDrawCmdType_2Di3, ptr, count)); //user logic error
        r = ScnVertexbuffs_v3IdxsInvalidate(opq->vbuffs, ptr, count);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//draw commands to consumer

ScnBOOL ScnModel2d_sendRenderCmds(ScnModel2dRef ref, const STScnGpuModelProps2D* const props, STScnModel2dPushItf* itf, void* itfParam){
    ScnBOOL r = ScnFALSE;
    STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    if(itf != NULL && itf->addCommandsWithProps != NULL){
        ScnMutex_lock(opq->mutex);
        {
            const STScnModel2dCmd* cmds = ScnArrayEmbed_getItmsPtr(&opq->cmds);
            const ScnUI32 cmdsSz = ScnArrayEmbed_getUse(&opq->cmds);
            //void* data, const STScnNode2dProps* props, const STScnModel2dCmd* cmds, const ScnUI32 cmdsSz
            r = (*itf->addCommandsWithProps)(itfParam, props, cmds, cmdsSz);
        }
        ScnMutex_unlock(opq->mutex);
    }
    return r;
}

