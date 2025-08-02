//
//  ScnModel2d.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 28/7/25.
//

#include "ixrender/scene/ScnModel2d.h"
#include "ixrender/core/ScnArray.h"

//STScnModel2dOpq

typedef struct STScnModel2dOpq {
    ScnContextRef       ctx;
    ScnMutexRef         mutex;
    //cmds
    struct {
        ScnVertexbuffsRef vbuffs;
        ScnBOOL             isHeap;
        union {
            //optimization, allow one command without allocating an array; most models will use one command.
            struct {
                ScnBOOL         isSet;
                STScnModel2dCmd cmd;
            } embedded;
            ScnArrayStruct(heap, STScnModel2dCmd);
        };
    } cmds;
} STScnModel2dOpq;

//

ScnUI32 ScnModel2d_getDrawCmdsCountLockedOpq_(STScnModel2dOpq* opq);
STScnModel2dCmd* ScnModel2d_getDrawCmdsPtrLockedOpq_(STScnModel2dOpq* opq);
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
        //
    }
}

void ScnModel2d_destroyOpq(void* obj){
    STScnModel2dOpq* opq = (STScnModel2dOpq*)obj;
    //cmds
    {
        if(!opq->cmds.isHeap){
            if(opq->cmds.embedded.isSet){
                ScnModel2dCmd_destroy(&opq->cmds.embedded.cmd);
                opq->cmds.embedded.isSet = ScnFALSE;
            }
        } else {
            ScnArray_foreach(&opq->cmds.heap, STScnModel2dCmd, c,
               ScnModel2dCmd_destroy(c);
            );
            ScnArray_destroy(opq->ctx, &opq->cmds.heap);
        }
        ScnVertexbuffs_releaseAndNull(&opq->cmds.vbuffs);
    }
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNull(&opq->ctx);
}

//

ScnUI32 ScnModel2d_getDrawCmdsCountLockedOpq_(STScnModel2dOpq* opq){
    return (opq->cmds.isHeap ? opq->cmds.heap.use : opq->cmds.embedded.isSet ? 1 : 0);
}

STScnModel2dCmd* ScnModel2d_getDrawCmdsPtrLockedOpq_(STScnModel2dOpq* opq){
    return (opq->cmds.isHeap ? opq->cmds.heap.arr : opq->cmds.embedded.isSet ? &opq->cmds.embedded.cmd : NULL);
}

//

ScnBOOL ScnModel2d_setVertexBuffs(ScnModel2dRef ref, ScnVertexbuffsRef vbuffs){
    ScnBOOL r = ScnFALSE;
    STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(0 == ScnModel2d_getDrawCmdsCountLockedOpq_(opq)){
        ScnVertexbuffs_set(&opq->cmds.vbuffs, vbuffs);
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
        if(!opq->cmds.isHeap){
            if(opq->cmds.embedded.isSet){
                ScnModel2dCmd_destroy(&opq->cmds.embedded.cmd);
                opq->cmds.embedded.isSet = ScnFALSE;
            }
        } else {
            ScnArray_foreach(&opq->cmds.heap, STScnModel2dCmd, c,
               ScnModel2dCmd_destroy(c);
            );
            ScnArray_empty(&opq->cmds.heap);
        }
    }
    ScnMutex_unlock(opq->mutex);
}

ScnBOOL ScnModel2d_addDrawCmdLockedOpq_(STScnModel2dOpq* opq, STScnModel2dCmd* cmd){
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
            STScnModel2dCmd cpy = opq->cmds.embedded.cmd;
            opq->cmds.isHeap = ScnTRUE;
            ScnArray_init(opq->ctx, &opq->cmds.heap, 0, 8, STScnModel2dCmd);
            if(NULL == ScnArray_addPtr(opq->ctx, &opq->cmds.heap, &cpy, STScnModel2dCmd)){
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
        if(NULL == ScnArray_addPtr(opq->ctx, &opq->cmds.heap, cmd, STScnModel2dCmd)){
            printf("ERROR, ScnArray_addPtr failed to add heap array itm.\n");
        } else {
            r = ScnTRUE;
        }
    }
    return r;
}

STScnVertex2DPtr ScnModel2d_addDraw(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 count){
    STScnVertex2DPtr r = STScnVertex2DPtr_Zero;
    if(shape >= ENScnRenderShape_Compute && shape < ENScnRenderShape_Count && count > 0){
        STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
        ScnMutex_lock(opq->mutex);
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v0Alloc(opq->cmds.vbuffs, count);
            if(r.ptr != NULL){
                STScnModel2dCmd cmd;
                ScnModel2dCmd_init(&cmd);
                ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
                cmd.shape       = shape;
                cmd.type        = ENScnModelDrawCmdType_2Dv0;
                cmd.verts.v0    = r;
                cmd.verts.count = count;
                if(!ScnModel2d_addDrawCmdLockedOpq_(opq, &cmd)){
                    printf("ERROR, ScnModel2d_addDrawCmdLockedOpq_ failed.\n");
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
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v1Alloc(opq->cmds.vbuffs, count);
            if(r.ptr != NULL){
                STScnModel2dCmd cmd;
                ScnModel2dCmd_init(&cmd);
                ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
                cmd.shape       = shape;
                cmd.type        = ENScnModelDrawCmdType_2Dv1;
                cmd.verts.v1    = r;
                cmd.verts.count = count;
                if(!ScnModel2d_addDrawCmdLockedOpq_(opq, &cmd)){
                    printf("ERROR, ScnModel2d_addDrawCmdLockedOpq_ failed.\n");
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
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v2Alloc(opq->cmds.vbuffs, count);
            if(r.ptr != NULL){
                STScnModel2dCmd cmd;
                ScnModel2dCmd_init(&cmd);
                ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
                cmd.shape       = shape;
                cmd.type        = ENScnModelDrawCmdType_2Dv2;
                cmd.verts.v2    = r;
                cmd.verts.count = count;
                if(!ScnModel2d_addDrawCmdLockedOpq_(opq, &cmd)){
                    printf("ERROR, ScnModel2d_addDrawCmdLockedOpq_ failed.\n");
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
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v3Alloc(opq->cmds.vbuffs, count);
            if(r.ptr != NULL){
                STScnModel2dCmd cmd;
                ScnModel2dCmd_init(&cmd);
                ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
                cmd.shape       = shape;
                cmd.type        = ENScnModelDrawCmdType_2Dv3;
                cmd.verts.v3    = r;
                cmd.verts.count = count;
                if(!ScnModel2d_addDrawCmdLockedOpq_(opq, &cmd)){
                    printf("ERROR, ScnModel2d_addDrawCmdLockedOpq_ failed.\n");
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
    STScnModel2dCmd* r = NULL;
    if(opq->cmds.isHeap){
        ScnArray_foreach(&opq->cmds.heap, STScnModel2dCmd, cmd,
            if(cmd->type == type && (void*)cmd->verts.v0.ptr <= (void*)ptrPtr && ((ScnBYTE*)ptrPtr + (sizePerItm * count)) <= ((ScnBYTE*)cmd->verts.v0.ptr + (sizePerItm * cmd->verts.count))){
                r = cmd;
                break;
            }
        );
    } else if(opq->cmds.embedded.isSet){
        STScnModel2dCmd* cmd = &opq->cmds.embedded.cmd;
        if(cmd->type == type && (void*)cmd->verts.v0.ptr <= (void*)ptrPtr && ((ScnBYTE*)ptrPtr + (sizePerItm * count)) <= ((ScnBYTE*)cmd->verts.v0.ptr + (sizePerItm * cmd->verts.count))){
            r = cmd;
        }
    }
    return r;
}

//Call these if you updated the vertices values after last render pass.

ScnBOOL ScnModel2d_v0FlagForSync(ScnModel2dRef ref, STScnVertex2DPtr ptr, const ScnUI32 count){
    ScnBOOL r = ScnFALSE;
    STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
        SCN_ASSERT(NULL != ScnModel2d_findDrawVertCmdLockedOpq_(opq, ENScnModelDrawCmdType_2Dv0, ptr.ptr, sizeof(ptr.ptr[0]), count)) //user logic error
        r = ScnVertexbuffs_v0Invalidate(opq->cmds.vbuffs, ptr, count);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnModel2d_v1FlagForSync(ScnModel2dRef ref, STScnVertex2DTexPtr ptr, const ScnUI32 count){
    ScnBOOL r = ScnFALSE;
    STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
        SCN_ASSERT(NULL != ScnModel2d_findDrawVertCmdLockedOpq_(opq, ENScnModelDrawCmdType_2Dv1, ptr.ptr, sizeof(ptr.ptr[0]), count)) //user logic error
        r = ScnVertexbuffs_v1Invalidate(opq->cmds.vbuffs, ptr, count);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnModel2d_v2FlagForSync(ScnModel2dRef ref, STScnVertex2DTex2Ptr ptr, const ScnUI32 count){
    ScnBOOL r = ScnFALSE;
    STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
        SCN_ASSERT(NULL != ScnModel2d_findDrawVertCmdLockedOpq_(opq, ENScnModelDrawCmdType_2Dv2, ptr.ptr, sizeof(ptr.ptr[0]), count)) //user logic error
        r = ScnVertexbuffs_v2Invalidate(opq->cmds.vbuffs, ptr, count);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnModel2d_v3FlagForSync(ScnModel2dRef ref, STScnVertex2DTex3Ptr ptr, const ScnUI32 count){
    ScnBOOL r = ScnFALSE;
    STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
        SCN_ASSERT(NULL != ScnModel2d_findDrawVertCmdLockedOpq_(opq, ENScnModelDrawCmdType_2Dv3, ptr.ptr, sizeof(ptr.ptr[0]), count)) //user logic error
        r = ScnVertexbuffs_v3Invalidate(opq->cmds.vbuffs, ptr, count);
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
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v0IdxsAlloc(opq->cmds.vbuffs, countIdxs);
            if(r.ptr != NULL){
                v = ScnVertexbuffs_v0Alloc(opq->cmds.vbuffs, countVerts);
                if(v.ptr == NULL){
                    ScnVertexbuffs_v0IdxsFree(opq->cmds.vbuffs, r);
                    r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                } else {
                    STScnModel2dCmd cmd;
                    ScnModel2dCmd_init(&cmd);
                    ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
                    cmd.shape       = shape;
                    cmd.type        = ENScnModelDrawCmdType_2Di0;
                    cmd.verts.v0    = v;
                    cmd.verts.count = countVerts;
                    cmd.idxs.i0     = r;
                    cmd.idxs.count  = countIdxs;
                    if(!ScnModel2d_addDrawCmdLockedOpq_(opq, &cmd)){
                        printf("ERROR, ScnModel2d_addDrawCmdLockedOpq_ failed.\n");
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
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v1IdxsAlloc(opq->cmds.vbuffs, countIdxs);
            if(r.ptr != NULL){
                v = ScnVertexbuffs_v1Alloc(opq->cmds.vbuffs, countVerts);
                if(v.ptr == NULL){
                    ScnVertexbuffs_v1IdxsFree(opq->cmds.vbuffs, r);
                    r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                } else {
                    STScnModel2dCmd cmd;
                    ScnModel2dCmd_init(&cmd);
                    ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
                    cmd.shape       = shape;
                    cmd.type        = ENScnModelDrawCmdType_2Di1;
                    cmd.verts.v1    = v;
                    cmd.verts.count = countVerts;
                    cmd.idxs.i1     = r;
                    cmd.idxs.count  = countIdxs;
                    if(!ScnModel2d_addDrawCmdLockedOpq_(opq, &cmd)){
                        printf("ERROR, ScnModel2d_addDrawCmdLockedOpq_ failed.\n");
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
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v2IdxsAlloc(opq->cmds.vbuffs, countIdxs);
            if(r.ptr != NULL){
                v = ScnVertexbuffs_v2Alloc(opq->cmds.vbuffs, countVerts);
                if(v.ptr == NULL){
                    ScnVertexbuffs_v2IdxsFree(opq->cmds.vbuffs, r);
                    r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                } else {
                    STScnModel2dCmd cmd;
                    ScnModel2dCmd_init(&cmd);
                    ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
                    cmd.shape       = shape;
                    cmd.type        = ENScnModelDrawCmdType_2Di2;
                    cmd.verts.v2    = v;
                    cmd.verts.count = countVerts;
                    cmd.idxs.i2     = r;
                    cmd.idxs.count  = countIdxs;
                    if(!ScnModel2d_addDrawCmdLockedOpq_(opq, &cmd)){
                        printf("ERROR, ScnModel2d_addDrawCmdLockedOpq_ failed.\n");
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
        if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
            r = ScnVertexbuffs_v3IdxsAlloc(opq->cmds.vbuffs, countIdxs);
            if(r.ptr != NULL){
                v = ScnVertexbuffs_v3Alloc(opq->cmds.vbuffs, countVerts);
                if(v.ptr == NULL){
                    ScnVertexbuffs_v3IdxsFree(opq->cmds.vbuffs, r);
                    r = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
                } else {
                    STScnModel2dCmd cmd;
                    ScnModel2dCmd_init(&cmd);
                    ScnVertexbuffs_set(&cmd.vbuffs, opq->cmds.vbuffs);
                    cmd.shape       = shape;
                    cmd.type        = ENScnModelDrawCmdType_2Di3;
                    cmd.verts.v3    = v;
                    cmd.verts.count = countVerts;
                    cmd.idxs.i3     = r;
                    cmd.idxs.count  = countIdxs;
                    if(!ScnModel2d_addDrawCmdLockedOpq_(opq, &cmd)){
                        printf("ERROR, ScnModel2d_addDrawCmdLockedOpq_ failed.\n");
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
    STScnModel2dCmd* r = NULL;
    if(opq->cmds.isHeap){
        ScnArray_foreach(&opq->cmds.heap, STScnModel2dCmd, cmd,
            if(cmd->type == type && cmd->idxs.i0.ptr <= ptr.ptr && (ptr.ptr + count) <= (cmd->idxs.i0.ptr + cmd->idxs.count)){
                r = cmd;
                break;
            }
        );
    } else if(opq->cmds.embedded.isSet){
        STScnModel2dCmd* cmd = &opq->cmds.embedded.cmd;
        if(cmd->type == type && cmd->idxs.i0.ptr <= ptr.ptr && (ptr.ptr + count) <= (cmd->idxs.i0.ptr + cmd->idxs.count)){
            r = cmd;
        }
    }
    return r;
}

//Call these if you updated the vertices values after last render pass.

ScnBOOL ScnModel2d_i0FlagForSync(ScnModel2dRef ref, STScnVertexIdxPtr ptr, const ScnUI32 count){
    ScnBOOL r = ScnFALSE;
    STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
        SCN_ASSERT(NULL != ScnModel2d_findDrawIdxCmdLockedOpq_(opq, ENScnModelDrawCmdType_2Di0, ptr, count)); //user logic error
        r = ScnVertexbuffs_v0IdxsInvalidate(opq->cmds.vbuffs, ptr, count);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnModel2d_i1FlagForSync(ScnModel2dRef ref, STScnVertexIdxPtr ptr, const ScnUI32 count){
    ScnBOOL r = ScnFALSE;
    STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
        SCN_ASSERT(NULL != ScnModel2d_findDrawIdxCmdLockedOpq_(opq, ENScnModelDrawCmdType_2Di1, ptr, count)); //user logic error
        r = ScnVertexbuffs_v1IdxsInvalidate(opq->cmds.vbuffs, ptr, count);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnModel2d_i2FlagForSync(ScnModel2dRef ref, STScnVertexIdxPtr ptr, const ScnUI32 count){
    ScnBOOL r = ScnFALSE;
    STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
        SCN_ASSERT(NULL != ScnModel2d_findDrawIdxCmdLockedOpq_(opq, ENScnModelDrawCmdType_2Di2, ptr, count)); //user logic error
        r = ScnVertexbuffs_v2IdxsInvalidate(opq->cmds.vbuffs, ptr, count);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnModel2d_i3FlagForSync(ScnModel2dRef ref, STScnVertexIdxPtr ptr, const ScnUI32 count){
    ScnBOOL r = ScnFALSE;
    STScnModel2dOpq* opq = (STScnModel2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnVertexbuffs_isNull(opq->cmds.vbuffs)){
        SCN_ASSERT(NULL != ScnModel2d_findDrawIdxCmdLockedOpq_(opq, ENScnModelDrawCmdType_2Di3, ptr, count)); //user logic error
        r = ScnVertexbuffs_v3IdxsInvalidate(opq->cmds.vbuffs, ptr, count);
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
            const STScnModel2dCmd* cmds = ScnModel2d_getDrawCmdsPtrLockedOpq_(opq);
            const ScnUI32 cmdsSz = ScnModel2d_getDrawCmdsCountLockedOpq_(opq);
            //void* data, const STScnNode2dProps* props, const STScnModel2dCmd* cmds, const ScnUI32 cmdsSz
            r = (*itf->addCommandsWithProps)(itfParam, props, cmds, cmdsSz);
        }
        ScnMutex_unlock(opq->mutex);
    }
    return r;
}

