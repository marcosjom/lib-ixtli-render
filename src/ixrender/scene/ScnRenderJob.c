//
//  ScnRenderJob.c
//  ixtli-render
//
//  Created by Marcos Ortega on 10/8/25.
//

#include "ixrender/scene/ScnRenderJob.h"
#include "ixrender/core/ScnArray.h"
#include "ixrender/core/ScnArraySorted.h"
#include "ixrender/scene/ScnRenderCmds.h"

//STScnRenderJobOpq

typedef struct STScnRenderJobOpq {
    ScnContextRef       ctx;
    ScnMutexRef         mutex;
    STScnRenderCmds     cmds;
    ScnGpuRenderJobRef  gpuJob;
} STScnRenderJobOpq;

//

ScnUI32 ScnRenderJob_getOpqSz(void){
    return (ScnUI32)sizeof(STScnRenderJobOpq);
}

void ScnRenderJob_initZeroedOpq(ScnContextRef ctx, void* obj) {
    STScnRenderJobOpq* opq = (STScnRenderJobOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex = ScnContext_allocMutex(opq->ctx);
    //
    ScnRenderCmds_init(ctx, &opq->cmds);
}

void ScnRenderJob_destroyOpq(void* obj){
    STScnRenderJobOpq* opq = (STScnRenderJobOpq*)obj;
    ScnRenderCmds_destroy(&opq->cmds);
    ScnGpuRenderJob_release(&opq->gpuJob);
    //
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNull(&opq->ctx);
}

//state

ENScnGpuRenderJobState ScnRenderJob_getState(ScnRenderJobRef ref){
    ENScnGpuRenderJobState r = ENScnGpuRenderJobState_Unknown;
    STScnRenderJobOpq* opq = (STScnRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnGpuRenderJob_isNull(opq->gpuJob)){
        r = ScnGpuRenderJob_getState(opq->gpuJob);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

// internal buffers (optimization to reduce memory allocations between render jobs)

ScnBOOL ScnRenderJob_swapCmds(ScnRenderJobRef ref, STScnRenderCmds* srcAndDstCmds, ScnGpuRenderJobRef* srcAndDestGpuJob){
    ScnBOOL r = ScnFALSE;
    STScnRenderJobOpq* opq = (STScnRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    //
    if(srcAndDstCmds == NULL || srcAndDestGpuJob == NULL){
        return ScnFALSE;
    }
    //
    ScnMutex_lock(opq->mutex);
    {
        STScnRenderCmds cmd = *srcAndDstCmds;
        ScnGpuRenderJobRef job = *srcAndDestGpuJob;
        *srcAndDstCmds      = opq->cmds;
        *srcAndDestGpuJob   = opq->gpuJob;
        opq->cmds           = cmd;
        opq->gpuJob         = job;
        r                   = ScnTRUE;
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//job framebuffers

ScnBOOL ScnRenderJob_framebuffPush(ScnRenderJobRef ref, ScnFramebuffRef fbuff){
    ScnBOOL r = ScnFALSE;
    STScnRenderJobOpq* opq = (STScnRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnMemElastic_isNull(opq->cmds.mPropsScns)){
        STScnRenderFbuffState* f = NULL;
        if(opq->cmds.stackUse < opq->cmds.stack.use){
            //reuse record
            f = &opq->cmds.stack.arr[opq->cmds.stackUse];
        } else {
            STScnRenderFbuffState ff;
            ScnRenderFbuffState_init(opq->ctx, &ff);
            f = ScnArray_addPtr(opq->ctx, &opq->cmds.stack, &ff, STScnRenderFbuffState);
            if(f == NULL){
                SCN_PRINTF_ERROR("ScnRenderJob_framebuffPush::ScnArray_addPtr failed.\n");
                ScnRenderFbuffState_destroy(&ff);
            }
        }
        //set initial state
        if(f != NULL){
            STScnGpuModelProps2d t = STScnGpuModelProps2d_Identity;
            STScnRenderFbuffProps props = STScnRenderFbuffProps_Zero;
            props.props = ScnFramebuff_getProps(fbuff);
            props.ptr   = ScnMemElastic_malloc(opq->cmds.mPropsScns, sizeof(STScnGpuFramebuffProps), NULL);
            //reset
            f->fbuff    = fbuff;
            ScnRenderFbuffState_reset(f);
            //
            if(props.ptr.ptr == NULL){
                SCN_PRINTF_ERROR("ScnRenderJob_framebuffPush::ScnMemElastic_malloc(STScnGpuFramebuffProps) failed.\n");
            } else if(!ScnRenderFbuffState_addProps(f, &props)){
                SCN_PRINTF_ERROR("ScnRenderJob_framebuffPush::ScnRenderFbuffState_addProps failed.\n");
            } else if(!ScnRenderFbuffState_addTransform(f, &t)){
                SCN_PRINTF_ERROR("ScnRenderJob_framebuffPush::ScnRenderFbuffState_addTransform failed.\n");
            } else if(!ScnRenderCmds_addUsedObj(&opq->cmds, ENScnRenderJobObjType_Framebuff, (ScnObjRef*)&fbuff, NULL)){
                SCN_PRINTF_ERROR("ScnRenderJob_framebuffPush::ScnRenderCmds_addUsedObj failed.\n");
            } else {
                //add cmd
                STScnRenderCmd cmd;
                cmd.cmdId                       = ENScnRenderCmd_ActivateFramebuff;
                cmd.activateFramebuff.ref       = fbuff;
                cmd.activateFramebuff.offset    = props.ptr.idx;
                cmd.activateFramebuff.viewport  = props.props.viewport;
                if(!ScnRenderCmds_add(&opq->cmds, &cmd)){
                    SCN_PRINTF_ERROR("ScnRenderJob_framebuffPush::ScnRenderCmds_add failed.\n");
                } else {
                    //push fb props
                    {
                        STScnGpuFramebuffProps* buffPtr = (STScnGpuFramebuffProps*)props.ptr.ptr;
                        *buffPtr = props.props;
                        opq->cmds.stackUse++;
                    }
                    r = ScnTRUE;
                }
            }
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRenderJob_framebuffPop(ScnRenderJobRef ref){
    ScnBOOL r = ScnFALSE;
    STScnRenderJobOpq* opq = (STScnRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->cmds.stackUse <= 0){
        SCN_PRINTF_ERROR("ScnRenderJob_framebuffPop::nothing to pop.\n");
    } else {
        r = ScnTRUE;
        opq->cmds.stackUse--;
        //reset prev fb active state (to force initial state)
        if(opq->cmds.stackUse > 0){
            STScnRenderFbuffState* f = &opq->cmds.stack.arr[opq->cmds.stackUse - 1];
            ScnRenderFbuffState_resetActiveState(f);
            //add cmd
            if(f->stacks.props.use <= 0){
                //program logic error (should be previously validated)
                SCN_PRINTF_ERROR("ScnRenderJob_framebuffPop::prev-framebuff has not properties in stack.\n");
                r = ScnFALSE;
            } else {
                const STScnRenderFbuffProps* props = &f->stacks.props.arr[f->stacks.props.use - 1];
                STScnRenderCmd cmd;
                cmd.cmdId                       = ENScnRenderCmd_ActivateFramebuff;
                cmd.activateFramebuff.ref       = f->fbuff;
                cmd.activateFramebuff.offset    = props->ptr.idx;
                cmd.activateFramebuff.viewport  = props->props.viewport;
                if(!ScnRenderCmds_add(&opq->cmds, &cmd)){
                    SCN_PRINTF_ERROR("ScnRenderJob_framebuffPop::ScnRenderCmds_add failed.\n");
                } else {
                    r = ScnFALSE;
                }
            }
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRenderJob_framebuffPropsPushLockedOpq_(STScnRenderJobOpq* opq, STScnRenderFbuffState* f, const STScnGpuFramebuffProps* pProps){
    ScnBOOL r = ScnFALSE;
    //add cmd
    STScnRenderFbuffProps props = STScnRenderFbuffProps_Zero;
    props.props = *pProps;
    props.ptr   = ScnMemElastic_malloc(opq->cmds.mPropsScns, sizeof(STScnGpuFramebuffProps), NULL);
    if(props.ptr.ptr == NULL){
        SCN_PRINTF_ERROR("ScnRenderJob_framebuffPropsPushLockedOpq_::ScnMemElastic_malloc(STScnGpuFramebuffProps) failed.\n");
    } else if(!ScnRenderFbuffState_addProps(f, &props)){
        SCN_PRINTF_ERROR("ScnRenderJob_framebuffPropsPushLockedOpq_::ScnRenderFbuffState_addProps failed.\n");
    } else {
        //add cmd
        STScnRenderCmd cmd;
        cmd.cmdId                       = ENScnRenderCmd_SetFramebuffProps;
        cmd.setFramebuffProps.offset    = props.ptr.idx;
        cmd.setFramebuffProps.viewport  = props.props.viewport;
        if(!ScnRenderCmds_add(&opq->cmds, &cmd)){
            SCN_PRINTF_ERROR("ScnRenderJob_framebuffPropsPushLockedOpq_::ScnRenderCmds_add failed.\n");
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

ScnBOOL ScnRenderJob_framebuffPropsPush(ScnRenderJobRef ref, const STScnGpuFramebuffProps pProps){
    ScnBOOL r = ScnFALSE;
    STScnRenderJobOpq* opq = (STScnRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->cmds.stackUse <= 0){
        SCN_PRINTF_ERROR("ScnRenderJob_framebuffPropsPush::no active framebuffer.\n");
    } else {
        STScnRenderFbuffState* f = &opq->cmds.stack.arr[opq->cmds.stackUse - 1];
        //add cmd
        if(f->stacks.props.use <= 0){
            //program logic error (should be previously validated)
            SCN_PRINTF_ERROR("ScnRenderJob_framebuffPropsPush::framebuff has not properties in stack.\n");
        } else {
            r = ScnRenderJob_framebuffPropsPushLockedOpq_(opq, f, &pProps);
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRenderJob_framebuffPropsOrthoPush(ScnRenderJobRef ref, const STScnAABBox3d ortho){
    ScnBOOL r = ScnFALSE;
    STScnRenderJobOpq* opq = (STScnRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->cmds.stackUse <= 0){
        SCN_PRINTF_ERROR("ScnRenderJob_framebuffPropsPush::no active framebuffer.\n");
    } else {
        STScnRenderFbuffState* f = &opq->cmds.stack.arr[opq->cmds.stackUse - 1];
        //add cmd
        if(f->stacks.props.use <= 0){
            //program logic error (should be previously validated)
            SCN_PRINTF_ERROR("ScnRenderJob_framebuffPropsPush::framebuff has not properties in stack.\n");
        } else {
            STScnGpuFramebuffProps propsPrev = f->stacks.props.arr[f->stacks.props.use - 1].props;
            propsPrev.ortho = ortho;
            r = ScnRenderJob_framebuffPropsPushLockedOpq_(opq, f, &propsPrev);
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRenderJob_framebuffPropsPop(ScnRenderJobRef ref){
    ScnBOOL r = ScnFALSE;
    STScnRenderJobOpq* opq = (STScnRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->cmds.stackUse <= 0){
        SCN_PRINTF_ERROR("ScnRenderJob_framebuffPropsPop::no active framebuffer.\n");
    } else {
        STScnRenderFbuffState* f = &opq->cmds.stack.arr[opq->cmds.stackUse - 1];
        //add cmd
        if(f->stacks.props.use <= 1){
            SCN_PRINTF_ERROR("ScnRenderJob_framebuffPropsPop::framebuff has not properties to pop.\n");
        } else {
            //add cmd
            const STScnRenderFbuffProps* propsPrev = &f->stacks.props.arr[f->stacks.props.use - 1];
            STScnRenderCmd cmd;
            cmd.cmdId                       = ENScnRenderCmd_SetFramebuffProps;
            cmd.setFramebuffProps.offset    = propsPrev->ptr.idx;
            cmd.setFramebuffProps.viewport  = propsPrev->props.viewport;
            if(!ScnRenderCmds_add(&opq->cmds, &cmd)){
                SCN_PRINTF_ERROR("ScnRenderJob_framebuffPropsPop::ScnRenderCmds_add failed.\n");
            } else {
                r = ScnTRUE;
            }
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//job textures

ScnBOOL ScnRenderJob_setTextureLockedOpq_(STScnRenderJobOpq* opq, STScnRenderFbuffState* f, const ENScnGpuTextureIdx idx, ScnTextureRef tex){
    ScnBOOL r = ScnFALSE, isFirstUse = ScnFALSE;
    if(ScnTexture_isSame(f->active.texs.refs[idx], tex)){
        SCN_PRINTF_VERB("Same texture, ignoring command.\n");
        r = ScnTRUE;
    } else if(!ScnRenderCmds_addUsedObj(&opq->cmds, ENScnRenderJobObjType_Texture, (ScnObjRef*)&tex, &isFirstUse)){
        SCN_PRINTF_ERROR("ScnRenderJob_setTexture::ScnRenderCmds_addUsedObj failed.\n");
    } else {
        //add cmd
        STScnRenderCmd cmd;
        cmd.cmdId               = ENScnRenderCmd_SetTexture;
        cmd.setTexture.ref      = tex;
        cmd.setTexture.index    = idx;
        cmd.setTexture.isFirstUse = isFirstUse;
        if(!ScnRenderCmds_add(&opq->cmds, &cmd)){
            SCN_PRINTF_ERROR("ScnRenderJob_setTexture::ScnRenderCmds_add failed.\n");
        } else {
            f->active.texs.refs[idx] = tex; //do not retain
            ++f->active.texs.changesSeq;
            r = ScnTRUE;
        }
    }
    return r;
}

ScnBOOL ScnRenderJob_setTexture(ScnRenderJobRef ref, const ENScnGpuTextureIdx idx, ScnTextureRef tex){
    ScnBOOL r = ScnFALSE;
    STScnRenderJobOpq* opq = (STScnRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->cmds.stackUse <= 0){ //cmds-queues are per-framebuffer
        SCN_PRINTF_ERROR("ScnRenderJob_setTexture::no active framebuffer.\n");
    } else if(idx < 0 || idx >= ENScnGpuTextureIdx_Count){
        SCN_PRINTF_ERROR("ScnRenderJob_setTexture::idx-out-of-bounds.\n");
    } else {
        STScnRenderFbuffState* f = &opq->cmds.stack.arr[opq->cmds.stackUse - 1];
        r = ScnRenderJob_setTextureLockedOpq_(opq, f, idx, tex);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}


//job models

SC_INLN ScnBOOL ScnRenderJob_addSetVertexBuffIfNecesaryLockedOpq_(STScnRenderJobOpq* opq, STScnRenderFbuffState* f, ScnVertexbuffRef vbuff){
    if(ScnVertexbuff_isSame(f->active.vbuff, vbuff)){
        //redundant command
        return ScnTRUE;
    }
    //new command
    ScnBOOL isFirstUse = ScnFALSE;
    if(!ScnRenderCmds_addUsedObj(&opq->cmds, ENScnRenderJobObjType_Vertexbuff, (ScnObjRef*)&vbuff, &isFirstUse)){
        SCN_PRINTF_ERROR("ScnRenderJob_addSetVertexBuffIfNecesaryLockedOpq_::ScnRenderCmds_addUsedObj failed.\n");
        return ScnFALSE;
    } else {
        STScnRenderCmd cmd;
        cmd.cmdId = ENScnRenderCmd_SetVertexBuff;
        cmd.setVertexBuff.ref = vbuff;
        cmd.setVertexBuff.isFirstUse = isFirstUse;
        if(!ScnRenderCmds_add(&opq->cmds, &cmd)){
            SCN_PRINTF_ERROR("ScnRenderJob_addSetVertexBuffIfNecesaryLockedOpq_::ScnRenderCmds_add failed.\n");
            return ScnFALSE;
        } else {
            f->active.vbuff = vbuff;
        }
    }
    return ScnTRUE;
}

#define ScnRenderJob_addCommandDrawVertsLockeOpq_(OPQ, FRAMEBUFF_STATE, DRAW_CMD, VERTEX_TYPE, VERTEX_TYPE_IDX) \
    if(ScnVertexbuffs_isNull((DRAW_CMD)->vbuffs)){ \
        SCN_PRINTF_ERROR("ScnRenderJob_model2dAdd_addCommandsWithPropsLockedOpq_::cmd without ScnVertexbuffs.\n"); \
        r = ScnFALSE; \
    } else { \
        if(!ScnRenderJob_addSetVertexBuffIfNecesaryLockedOpq_(OPQ, FRAMEBUFF_STATE, ScnVertexbuffs_getVertexBuff((DRAW_CMD)->vbuffs, VERTEX_TYPE))){ \
            SCN_PRINTF_ERROR("ScnRenderJob_addSetVertexBuffIfNecesaryLockedOpq_ failed.\n"); \
            r = ScnFALSE; \
        } else { \
            STScnRenderCmd cmd; \
            cmd.cmdId               = ENScnRenderCmd_DrawVerts; \
            cmd.drawVerts.shape     = (DRAW_CMD)->shape; \
            cmd.drawVerts.iFirst    = (DRAW_CMD)->verts.v ## VERTEX_TYPE_IDX.idx; \
            cmd.drawVerts.count     = (DRAW_CMD)->verts.count; \
            if(!ScnRenderCmds_add(&(OPQ)->cmds, &cmd)){ \
                SCN_PRINTF_ERROR("ScnRenderJob_addSetVertexBuffIfNecesaryLockedOpq_::ScnRenderCmds_add failed.\n"); \
                r = ScnFALSE; \
            } \
        } \
    }

#define ScnRenderJob_addCommandDrawIndexesLockeOpq_(OPQ, FRAMEBUFF_STATE, DRAW_CMD, VERTEX_TYPE, VERTEX_TYPE_IDX) \
    if(ScnVertexbuffs_isNull((DRAW_CMD)->vbuffs)){ \
        SCN_PRINTF_ERROR("ScnRenderJob_model2dAdd_addCommandsWithPropsLockedOpq_::cmd without ScnVertexbuffs.\n"); \
        r = ScnFALSE; \
    } else { \
        if(!ScnRenderJob_addSetVertexBuffIfNecesaryLockedOpq_(OPQ, FRAMEBUFF_STATE, ScnVertexbuffs_getVertexBuff((DRAW_CMD)->vbuffs, VERTEX_TYPE))){ \
            SCN_PRINTF_ERROR("ScnRenderJob_addSetVertexBuffIfNecesaryLockedOpq_ failed.\n"); \
            r = ScnFALSE; \
        } else { \
            STScnRenderCmd cmd; \
            cmd.cmdId               = ENScnRenderCmd_DrawIndexes; \
            cmd.drawIndexes.shape   = (DRAW_CMD)->shape; \
            cmd.drawIndexes.iFirst  = (DRAW_CMD)->idxs.i ## VERTEX_TYPE_IDX.idx; \
            cmd.drawIndexes.count   = (DRAW_CMD)->idxs.count; \
            if(!ScnRenderCmds_add(&(OPQ)->cmds, &cmd)){ \
                SCN_PRINTF_ERROR("ScnRenderJob_addSetVertexBuffIfNecesaryLockedOpq_::ScnRenderCmds_add failed.\n"); \
                r = ScnFALSE; \
            } \
        } \
    }

STScnAbsPtr ScnRenderJob_model2dAdd_addGpuModelPropsLockedOpq_(STScnRenderJobOpq* opq, STScnRenderFbuffState* f, const STScnGpuModelProps2d* const transform){
    STScnAbsPtr r = ScnMemElastic_malloc(opq->cmds.mPropsMdls, sizeof(STScnGpuModelProps2d), NULL);
    if(r.ptr == NULL){
        SCN_PRINTF_ERROR("ScnRenderJob_model2dAdd_addCommandsWithTransformLockedOpq_::ScnMemElastic_malloc failed.\n");
    } else {
        STScnRenderCmd cmd;
        cmd.cmdId = ENScnRenderCmd_SetTransformOffset;
        cmd.setTransformOffset.offset = r.idx;
        if(!ScnRenderCmds_add(&opq->cmds, &cmd)){
            SCN_PRINTF_ERROR("ScnRenderJob_model2dAdd_addCommandsWithTransformLockedOpq_::ScnRenderCmds_add failed.\n");
            r.ptr = NULL;
        } else {
            STScnGpuModelProps2d* curProps = (STScnGpuModelProps2d*)r.ptr;
            *curProps = *transform;
            if(opq->cmds.gpuDevDesc.isTexFmtInfoRequired){
                curProps->texs.fmts[0] = ScnTexture_isNull(f->active.texs.refs[0]) ? ENScnBitmapColor_undef : ScnTexture_getImageProps(f->active.texs.refs[0], NULL).color;
                curProps->texs.fmts[1] = ScnTexture_isNull(f->active.texs.refs[1]) ? ENScnBitmapColor_undef : ScnTexture_getImageProps(f->active.texs.refs[1], NULL).color;
                curProps->texs.fmts[2] = ScnTexture_isNull(f->active.texs.refs[2]) ? ENScnBitmapColor_undef : ScnTexture_getImageProps(f->active.texs.refs[2], NULL).color;
            }
        }
    }
    return r;
}

ScnBOOL ScnRenderJob_model2dAdd_addCommandsWithTransformLockedOpq_(STScnRenderJobOpq* opq, STScnRenderFbuffState* f, const STScnGpuModelProps2d* const transform, const STScnModel2dCmd* const cmds, const ScnUI32 cmdsSz){
    ScnBOOL r = ScnFALSE;
    //add commands with this matrix (do not push it into the stack)
    STScnAbsPtr tNPtr = ScnRenderJob_model2dAdd_addGpuModelPropsLockedOpq_(opq, f, transform);
    if(tNPtr.ptr == NULL){
        SCN_PRINTF_ERROR("ScnRenderJob_model2dAdd_addGpuModelPropsLockedOpq_::ScnRenderJob_model2dAdd_addGpuModelPropsLockedOpq_ failed.\n");
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
                        ScnRenderJob_addCommandDrawVertsLockeOpq_(opq, f, c, ENScnVertexType_2DColor, 0);
                        break;
                    case ENScnModelDrawCmdType_2Dv1:
                        //sync textures
                        if(!ScnRenderJob_setTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_0, c->texs[0])){
                            r = ScnFALSE;
                        }
                        //inject new obj-props if textures changed and their information is required by the device
                        if(opq->cmds.gpuDevDesc.isTexFmtInfoRequired && texChangesVer != f->active.texs.changesSeq && NULL == (tNPtr = ScnRenderJob_model2dAdd_addGpuModelPropsLockedOpq_(opq, f, transform)).ptr){
                            SCN_PRINTF_ERROR("ScnRenderJob_model2dAdd_addCommandsWithTransformLockedOpq_::ScnRenderJob_model2dAdd_addGpuModelPropsLockedOpq_ failed.\n");
                            r = ScnFALSE;
                            break;
                        } else {
                            texChangesVer = f->active.texs.changesSeq;
                        }
                        //draw command
                        ScnRenderJob_addCommandDrawVertsLockeOpq_(opq, f, c, ENScnVertexType_2DTex, 1);
                        break;
                    case ENScnModelDrawCmdType_2Dv2:
                        //sync textures
                        if(!ScnRenderJob_setTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_0, c->texs[0])){
                            r = ScnFALSE;
                        }
                        if(!ScnRenderJob_setTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_1, c->texs[1])){
                            r = ScnFALSE;
                        }
                        //inject new obj-props if textures changed and their information is required by the device
                        if(opq->cmds.gpuDevDesc.isTexFmtInfoRequired && texChangesVer != f->active.texs.changesSeq && NULL == (tNPtr = ScnRenderJob_model2dAdd_addGpuModelPropsLockedOpq_(opq, f, transform)).ptr){
                            SCN_PRINTF_ERROR("ScnRenderJob_model2dAdd_addCommandsWithTransformLockedOpq_::ScnRenderJob_model2dAdd_addGpuModelPropsLockedOpq_ failed.\n");
                            r = ScnFALSE;
                            break;
                        } else {
                            texChangesVer = f->active.texs.changesSeq;
                        }
                        //draw command
                        ScnRenderJob_addCommandDrawVertsLockeOpq_(opq, f, c, ENScnVertexType_2DTex2, 2);
                        break;
                    case ENScnModelDrawCmdType_2Dv3:
                        //sync textures
                        if(!ScnRenderJob_setTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_0, c->texs[0])){
                            r = ScnFALSE;
                        }
                        if(!ScnRenderJob_setTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_1, c->texs[1])){
                            r = ScnFALSE;
                        }
                        if(!ScnRenderJob_setTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_2, c->texs[2])){
                            r = ScnFALSE;
                        }
                        //inject new obj-props if textures changed and their information is required by the device
                        if(opq->cmds.gpuDevDesc.isTexFmtInfoRequired && texChangesVer != f->active.texs.changesSeq && NULL == (tNPtr = ScnRenderJob_model2dAdd_addGpuModelPropsLockedOpq_(opq, f, transform)).ptr){
                            SCN_PRINTF_ERROR("ScnRenderJob_model2dAdd_addCommandsWithTransformLockedOpq_::ScnRenderJob_model2dAdd_addGpuModelPropsLockedOpq_ failed.\n");
                            r = ScnFALSE;
                            break;
                        } else {
                            texChangesVer = f->active.texs.changesSeq;
                        }
                        //draw command
                        ScnRenderJob_addCommandDrawVertsLockeOpq_(opq, f, c, ENScnVertexType_2DTex3, 3);
                        break;
                        //
                    case ENScnModelDrawCmdType_2Di0:
                        //draw command
                        ScnRenderJob_addCommandDrawIndexesLockeOpq_(opq, f, c, ENScnVertexType_2DColor, 0);
                        break;
                    case ENScnModelDrawCmdType_2Di1:
                        //sync textures
                        if(!ScnRenderJob_setTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_0, c->texs[0])){
                            r = ScnFALSE;
                        }
                        //inject new obj-props if textures changed and their information is required by the device
                        if(opq->cmds.gpuDevDesc.isTexFmtInfoRequired && texChangesVer != f->active.texs.changesSeq && NULL == (tNPtr = ScnRenderJob_model2dAdd_addGpuModelPropsLockedOpq_(opq, f, transform)).ptr){
                            SCN_PRINTF_ERROR("ScnRenderJob_model2dAdd_addCommandsWithTransformLockedOpq_::ScnRenderJob_model2dAdd_addGpuModelPropsLockedOpq_ failed.\n");
                            r = ScnFALSE;
                            break;
                        } else {
                            texChangesVer = f->active.texs.changesSeq;
                        }
                        //draw command
                        ScnRenderJob_addCommandDrawIndexesLockeOpq_(opq, f, c, ENScnVertexType_2DTex, 1);
                        break;
                    case ENScnModelDrawCmdType_2Di2:
                        //sync textures
                        if(!ScnRenderJob_setTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_0, c->texs[0])){
                            r = ScnFALSE;
                        }
                        if(!ScnRenderJob_setTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_1, c->texs[1])){
                            r = ScnFALSE;
                        }
                        //inject new obj-props if textures changed and their information is required by the device
                        if(opq->cmds.gpuDevDesc.isTexFmtInfoRequired && texChangesVer != f->active.texs.changesSeq && NULL == (tNPtr = ScnRenderJob_model2dAdd_addGpuModelPropsLockedOpq_(opq, f, transform)).ptr){
                            SCN_PRINTF_ERROR("ScnRenderJob_model2dAdd_addCommandsWithTransformLockedOpq_::ScnRenderJob_model2dAdd_addGpuModelPropsLockedOpq_ failed.\n");
                            r = ScnFALSE;
                            break;
                        } else {
                            texChangesVer = f->active.texs.changesSeq;
                        }
                        //draw command
                        ScnRenderJob_addCommandDrawIndexesLockeOpq_(opq, f, c, ENScnVertexType_2DTex2, 2);
                        break;
                    case ENScnModelDrawCmdType_2Di3:
                        //sync textures
                        if(!ScnRenderJob_setTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_0, c->texs[0])){
                            r = ScnFALSE;
                        }
                        if(!ScnRenderJob_setTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_1, c->texs[1])){
                            r = ScnFALSE;
                        }
                        if(!ScnRenderJob_setTextureLockedOpq_(opq, f, ENScnGpuTextureIdx_2, c->texs[2])){
                            r = ScnFALSE;
                        }
                        //inject new obj-props if textures changed and their information is required by the device
                        if(opq->cmds.gpuDevDesc.isTexFmtInfoRequired && texChangesVer != f->active.texs.changesSeq && NULL == (tNPtr = ScnRenderJob_model2dAdd_addGpuModelPropsLockedOpq_(opq, f, transform)).ptr){
                            SCN_PRINTF_ERROR("ScnRenderJob_model2dAdd_addCommandsWithTransformLockedOpq_::ScnRenderJob_model2dAdd_addGpuModelPropsLockedOpq_ failed.\n");
                            r = ScnFALSE;
                            break;
                        } else {
                            texChangesVer = f->active.texs.changesSeq;
                        }
                        //draw command
                        ScnRenderJob_addCommandDrawIndexesLockeOpq_(opq, f, c, ENScnVertexType_2DTex3, 3);
                        break;
                    default:
                        SCN_ASSERT(ScnFALSE) //missing implementation
                        break;
                }
                ++c;
            }
        }
    }
    return r;
}
    
ScnBOOL ScnRenderJob_model2dAdd_addCommandsWithPropsLockedOpq_(void* data, const STScnGpuModelProps2d* const props, const STScnModel2dCmd* const cmds, const ScnUI32 cmdsSz){
    ScnBOOL r = ScnFALSE;
    STScnRenderJobOpq* opq = (STScnRenderJobOpq*)data;
    if(opq != NULL && props != NULL){
        if(opq->cmds.stackUse <= 0){
            SCN_PRINTF_ERROR("ScnRenderJob_model2dAdd_addCommandsWithPropsLockedOpq_::no active framebuffer.\n");
        } else {
            STScnRenderFbuffState* f = &opq->cmds.stack.arr[opq->cmds.stackUse - 1];
            if(f->stacks.transforms.use < 1){
                SCN_PRINTF_ERROR("ScnRenderJob_model2dAdd_addCommandsWithPropsLockedOpq_::missing parent transform.\n");
            } else {
                SCN_ASSERT(!ScnMemElastic_isNull(opq->cmds.mPropsScns)) //program logic error
                SCN_ASSERT(!ScnMemElastic_isNull(opq->cmds.mPropsMdls)) //program logic error
                if(!ScnRenderJob_model2dAdd_addCommandsWithTransformLockedOpq_(opq, f, props, cmds, cmdsSz)){
                    SCN_PRINTF_ERROR("ScnRenderJob_model2dAdd_addCommandsWithPropsLockedOpq_::ScnRenderJob_model2dAdd_addCommandsWithTransformLockedOpq_ failed.\n");
                } else {
                    r = ScnTRUE;
                }
            }
        }
    }
    return r;
}

//job node (scene tree)

ScnBOOL ScnRenderJob_node2dPush(ScnRenderJobRef ref, ScnNode2dRef node){
    return ScnRenderJob_node2dPropsPushWithMode(ref, ScnNode2d_getProps(node), ScnRenderJobPushMode_Multiply);
}

ScnBOOL ScnRenderJob_node2dPushWithMode(ScnRenderJobRef ref, ScnNode2dRef node, const ScnRenderJobPushMode mode){
    return ScnRenderJob_node2dPropsPushWithMode(ref, ScnNode2d_getProps(node), mode);
}

ScnBOOL ScnRenderJob_node2dPropsPush(ScnRenderJobRef ref, const STScnNode2dProps nodeProps){
    return ScnRenderJob_node2dPropsPushWithMode(ref, nodeProps, ScnRenderJobPushMode_Multiply);
}

ScnBOOL ScnRenderJob_node2dPropsPushWithMode(ScnRenderJobRef ref, const STScnNode2dProps nodeProps, const ScnRenderJobPushMode mode){
    ScnBOOL r = ScnFALSE;
    STScnRenderJobOpq* opq = (STScnRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->cmds.stackUse <= 0){
        SCN_PRINTF_ERROR("ScnRenderJob_node2dPropsPush::no active framebuffer.\n");
    } else {
        SCN_ASSERT(!ScnMemElastic_isNull(opq->cmds.mPropsScns)) //program logic error
        SCN_ASSERT(!ScnMemElastic_isNull(opq->cmds.mPropsMdls)) //program logic error
        const STScnGpuModelProps2d t = ScnNode2dProps_toGpuTransform(&nodeProps);
        STScnRenderFbuffState* f = &opq->cmds.stack.arr[opq->cmds.stackUse - 1];
        switch (mode) {
            case ScnRenderJobPushMode_Multiply:
                if(f->stacks.transforms.use < 1){
                    SCN_PRINTF_ERROR("ScnRenderJob_node2dPropsPush::missing parent transform.\n");
                } else {
                    const STScnGpuModelProps2d tPrnt = f->stacks.transforms.arr[f->stacks.transforms.use - 1];
                    const STScnGpuModelProps2d tN = ScnGpuModelProps2d_multiply(&tPrnt, &t);
                    if(!ScnRenderFbuffState_addTransform(f, &tN)){
                        SCN_PRINTF_ERROR("ScnRenderJob_node2dPropsPush::ScnRenderFbuffState_addTransform failed.\n");
                    } else {
                        //ToDo: add render props to buffer
                        r = ScnTRUE;
                    }
                }
                break;
            case ScnRenderJobPushMode_Set:
                {
                    if(!ScnRenderFbuffState_addTransform(f, &t)){
                        SCN_PRINTF_ERROR("ScnRenderJob_node2dPropsPush::ScnRenderFbuffState_addTransform failed.\n");
                    } else {
                        //ToDo: add render props to buffer
                        r = ScnTRUE;
                    }
                }
                break;
            default:
                SCN_ASSERT(ScnFALSE)
                break;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRenderJob_node2dPop(ScnRenderJobRef ref){
    ScnBOOL r = ScnFALSE;
    STScnRenderJobOpq* opq = (STScnRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->cmds.stackUse <= 0){
        SCN_PRINTF_ERROR("ScnRenderJob_node2dPop::no framebuffer is active.\n");
    } else {
        SCN_ASSERT(!ScnMemElastic_isNull(opq->cmds.mPropsScns)) //program logic error
        SCN_ASSERT(!ScnMemElastic_isNull(opq->cmds.mPropsMdls)) //program logic error
        STScnRenderFbuffState* f = &opq->cmds.stack.arr[opq->cmds.stackUse - 1];
        if(f->stacks.transforms.use < 2){
            SCN_PRINTF_ERROR("ScnRenderJob_node2dPop::nothing to pop.\n");
        } else {
            f->stacks.transforms.use--;
            r = ScnTRUE;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//job models

ScnBOOL ScnRenderJob_model2dAdd(ScnRenderJobRef ref, ScnModel2dRef model){
    ScnBOOL r = ScnFALSE;
    STScnRenderJobOpq* opq = (STScnRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->cmds.stackUse <= 0){
        SCN_PRINTF_ERROR("ScnRenderJob_model2dAdd::no framebuffer is active.\n");
    } else {
        SCN_ASSERT(!ScnMemElastic_isNull(opq->cmds.mPropsScns)) //program logic error
        SCN_ASSERT(!ScnMemElastic_isNull(opq->cmds.mPropsMdls)) //program logic error
        STScnRenderFbuffState* f = &opq->cmds.stack.arr[opq->cmds.stackUse - 1];
        if(f->stacks.transforms.use < 1){
            SCN_PRINTF_ERROR("ScnRenderJob_model2dAdd::render props unavailable.\n");
        } else {
            SCN_ASSERT(!ScnMemElastic_isNull(opq->cmds.mPropsScns)) //program logic error
            SCN_ASSERT(!ScnMemElastic_isNull(opq->cmds.mPropsMdls)) //program logic error
            const STScnGpuModelProps2d tPrnt = f->stacks.transforms.arr[f->stacks.transforms.use - 1];
            if(!ScnModel2d_sendRenderCmds(model, &tPrnt, ScnRenderJob_model2dAdd_addCommandsWithPropsLockedOpq_, opq)){
                SCN_PRINTF_ERROR("ScnRenderJob_framebuffPop::ScnModel2d_sendRenderCmds failed.\n");
            } else {
                r = ScnTRUE;
            }
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRenderJob_model2dAddWithNode(ScnRenderJobRef ref, ScnModel2dRef model, ScnNode2dRef node){ //equivalent to jobNode2dPush() + jobModelAdd() + jobNodePropsPop()
    return !ScnModel2d_isNull(model) && !ScnNode2d_isNull(node) ? ScnRenderJob_model2dAddWithNodePropsAndMode(ref, model, ScnNode2d_getProps(node), ScnRenderJobPushMode_Multiply) : ScnFALSE;
}

ScnBOOL ScnRenderJob_model2dAddWithNodeAndMode(ScnRenderJobRef ref, ScnModel2dRef model, ScnNode2dRef node, const ScnRenderJobPushMode mode){
    return !ScnModel2d_isNull(model) && !ScnNode2d_isNull(node) ? ScnRenderJob_model2dAddWithNodePropsAndMode(ref, model, ScnNode2d_getProps(node), mode) : ScnFALSE;
}

ScnBOOL ScnRenderJob_model2dAddWithNodeProps(ScnRenderJobRef ref, ScnModel2dRef model, const STScnNode2dProps nodeProps){ //equivalent to jobNodePropsPush() + jobModelAdd() + jobNodePropsPop()
    return ScnRenderJob_model2dAddWithNodePropsAndMode(ref, model, nodeProps, ScnRenderJobPushMode_Multiply);
}

ScnBOOL ScnRenderJob_model2dAddWithNodePropsAndMode(ScnRenderJobRef ref, ScnModel2dRef model, const STScnNode2dProps nodeProps, const ScnRenderJobPushMode mode){
    ScnBOOL r = ScnFALSE;
    STScnRenderJobOpq* opq = (STScnRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->cmds.stackUse <= 0){
        SCN_PRINTF_ERROR("ScnRenderJob_model2dAddWithNodeProps::no active framebuffer.\n");
    } else {
        SCN_ASSERT(!ScnMemElastic_isNull(opq->cmds.mPropsScns)) //program logic error
        SCN_ASSERT(!ScnMemElastic_isNull(opq->cmds.mPropsMdls)) //program logic error
        const STScnGpuModelProps2d t = ScnNode2dProps_toGpuTransform(&nodeProps);
        STScnRenderFbuffState* f = &opq->cmds.stack.arr[opq->cmds.stackUse - 1];
        switch(mode){
            case ScnRenderJobPushMode_Multiply:
                if(f->stacks.transforms.use < 1){
                    SCN_PRINTF_ERROR("ScnRenderJob_model2dAddWithNodeProps::missing parent transform.\n");
                } else {
                    const STScnGpuModelProps2d tPrnt = f->stacks.transforms.arr[f->stacks.transforms.use - 1];
                    const STScnGpuModelProps2d tM = ScnGpuModelProps2d_multiply(&tPrnt, &t);
                    if(!ScnModel2d_sendRenderCmds(model, &tM, ScnRenderJob_model2dAdd_addCommandsWithPropsLockedOpq_, opq)){
                        SCN_PRINTF_ERROR("ScnRenderJob_model2dAddWithNodeProps::ScnModel2d_sendRenderCmds failed.\n");
                    } else {
                        r = ScnTRUE;
                    }
                }
                break;
            case ScnRenderJobPushMode_Set:
                if(!ScnModel2d_sendRenderCmds(model, &t, ScnRenderJob_model2dAdd_addCommandsWithPropsLockedOpq_, opq)){
                    SCN_PRINTF_ERROR("ScnRenderJob_model2dAddWithNodeProps::ScnModel2d_sendRenderCmds failed.\n");
                } else {
                    r = ScnTRUE;
                }
                break;
            default:
                SCN_ASSERT(ScnFALSE)
                break;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//job cmds

/*
void ScnRenderJob_cmdMaskModePush(ScnRenderJobRef ref){
    STScnRenderJobOpq* opq = (STScnRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnRenderCmd cmd;
        cmd.cmdId = ENScnRenderCmd_MaskModePush;
        ScnArray_addPtr(opq->ctx, &opq->cmds.cmds, &cmd, STScnRenderCmd);
    }
    ScnMutex_unlock(opq->mutex);
}

void ScnRenderJob_cmdMaskModePop(ScnRenderJobRef ref){
    STScnRenderJobOpq* opq = (STScnRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnRenderCmd cmd;
        cmd.cmdId = ENScnRenderCmd_MaskModePop;
        ScnArray_addPtr(opq->ctx, &opq->cmds.cmds, &cmd, STScnRenderCmd);
    }
    ScnMutex_unlock(opq->mutex);
}

void ScnRenderJob_cmdSetTexture(ScnRenderJobRef ref, const ScnUI32 index, ScnTextureRef tex){
    STScnRenderJobOpq* opq = (STScnRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnRenderCmd cmd;
        cmd.cmdId = ENScnRenderCmd_SetTexture;
        cmd.setTexture.index = index;
        cmd.setTexture.tex = tex;
        ScnArray_addPtr(opq->ctx, &opq->cmds.cmds, &cmd, STScnRenderCmd);
    }
    ScnMutex_unlock(opq->mutex);
}

void ScnRenderJob_cmdDawVerts(ScnRenderJobRef ref, const ENScnRenderShape mode, const ScnUI32 iFirst, const ScnUI32 count){
    STScnRenderJobOpq* opq = (STScnRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnRenderCmd cmd;
        cmd.cmdId = ENScnRenderCmd_DrawVerts;
        cmd.drawVerts.mode      = mode;
        cmd.drawVerts.iFirst    = iFirst;
        cmd.drawVerts.count     = count;
        ScnArray_addPtr(opq->ctx, &opq->cmds.cmds, &cmd, STScnRenderCmd);
    }
    ScnMutex_unlock(opq->mutex);
}

void ScnRenderJob_cmdDawIndexes(ScnRenderJobRef ref, const ENScnRenderShape mode, const ScnUI32 iFirst, const ScnUI32 count){
    STScnRenderJobOpq* opq = (STScnRenderJobOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        STScnRenderCmd cmd;
        cmd.cmdId = ENScnRenderCmd_DrawIndexes;
        cmd.drawVerts.mode      = mode;
        cmd.drawVerts.iFirst    = iFirst;
        cmd.drawVerts.count     = count;
        ScnArray_addPtr(opq->ctx, &opq->cmds.cmds, &cmd, STScnRenderCmd);
    }
    ScnMutex_unlock(opq->mutex);
}
*/

