//
//  ScnApiMetalRenderJob.m
//  ixtli-render
//
//  Created by Marcos Ortega on 18/8/25.
//

#include "ScnApiMetalRenderJob.h"
#include "ixrender/scene/ScnRenderCmd.h"
//
#include "ScnApiMetalVertexbuff.h"
#include "ScnApiMetalTexture.h"
#include "ScnApiMetalSampler.h"

//render job

void ScnApiMetalRenderJob_free(void* data){
    STScnApiMetalRenderJob* obj = (STScnApiMetalRenderJob*)data;
    ScnContextRef ctx = obj->ctx;
    {
        if(obj->cmdsBuff != nil){
#           ifdef SCN_ASSERTS_ACTIVATED
            const MTLCommandBufferStatus status = [obj->cmdsBuff status];
            SCN_ASSERT(status == MTLCommandBufferStatusNotEnqueued || status == MTLCommandBufferStatusCompleted || status == MTLCommandBufferStatusError) //should not be active
#           endif
            [obj->cmdsBuff release];
            obj->cmdsBuff = nil;
        }
        //bPropsScns
        {
            ScnGpuBuffer_releaseAndNull(&obj->bPropsScns.ref);
            obj->bPropsScns.obj = NULL;
        }
        //bPropsMdls
        {
            ScnGpuBuffer_releaseAndNull(&obj->bPropsMdls.ref);
            obj->bPropsMdls.obj = NULL;
        }
        ScnApiMetalRenderJobState_destroy(&obj->state);
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

ENScnGpuRenderJobState ScnApiMetalRenderJob_getState(void* data){
    ENScnGpuRenderJobState r = ENScnGpuRenderJobState_Unknown;
    STScnApiMetalRenderJob* obj = (STScnApiMetalRenderJob*)data;
    if(obj->cmdsBuff != nil){
        switch ([obj->cmdsBuff status]) {
            case MTLCommandBufferStatusNotEnqueued: ; break;
            case MTLCommandBufferStatusEnqueued: r = ENScnGpuRenderJobState_Enqueued; break;
            case MTLCommandBufferStatusCommitted: r = ENScnGpuRenderJobState_Enqueued; break;
            case MTLCommandBufferStatusScheduled: r = ENScnGpuRenderJobState_Enqueued; break;
            case MTLCommandBufferStatusCompleted: r = ENScnGpuRenderJobState_Completed; break;
            case MTLCommandBufferStatusError: r = ENScnGpuRenderJobState_Error; break;
            default: break;
        }
    }
    return r;
}

ScnBOOL ScnApiMetalRenderJob_buildBegin(void* data, ScnGpuBufferRef pBuffPropsScns, ScnGpuBufferRef pBuffPropsMdls){
    ScnBOOL r = ScnFALSE;
    STScnApiMetalRenderJob* obj = (STScnApiMetalRenderJob*)data;
    //
    STScnApiMetalBuffer* bPropsScns = NULL;
    STScnApiMetalBuffer* bPropsMdls = NULL;
    //
    if(obj->dev == NULL || obj->dev->dev == nil || obj->dev->cmdQueue == nil){
        SCN_PRINTF_ERROR("ScnApiMetalRenderJob_buildBegin::dev-or-cmdQueue is NULL.\n");
        return ScnFALSE;
    }
    if(ScnGpuBuffer_isNull(pBuffPropsScns) || ScnGpuBuffer_isNull(pBuffPropsMdls)){
        //SCN_PRINTF_ERROR("ScnApiMetalRenderJob_buildBegin::pBuffPropsScns-or-pBuffPropsMdls is NULL.\n");
        return ScnFALSE;
    }
    bPropsScns = (STScnApiMetalBuffer*)ScnGpuBuffer_getApiItfParam(pBuffPropsScns);
    bPropsMdls = (STScnApiMetalBuffer*)ScnGpuBuffer_getApiItfParam(pBuffPropsMdls);
    if(bPropsScns == NULL || bPropsScns->buff == nil){
        SCN_PRINTF_ERROR("ScnApiMetalRenderJob_buildBegin::bPropsScns is NULL.\n");
        return ScnFALSE;
    } else if(bPropsMdls == NULL || bPropsMdls->buff == nil){
        SCN_PRINTF_ERROR("ScnApiMetalRenderJob_buildBegin::bPropsMdls is NULL.\n");
        return ScnFALSE;
    }
    if(obj->cmdsBuff != nil){
        const MTLCommandBufferStatus status = [obj->cmdsBuff status];
        if(status != MTLCommandBufferStatusNotEnqueued && status != MTLCommandBufferStatusCompleted && status != MTLCommandBufferStatusError){
            //job currentyl in progress
            SCN_PRINTF_ERROR("ScnApiMetalRenderJob_buildBegin::[obj->cmdsBuff status] is active.\n");
            return ScnFALSE;
        }
    }
    //begin
    {
        if(obj->cmdsBuff != nil){
            [obj->cmdsBuff release];
            obj->cmdsBuff = nil;
        }
        obj->cmdsBuff = [obj->dev->cmdQueue commandBuffer];
        if(obj->cmdsBuff != nil){
            obj->cmdsBuff.label = @"Ixtli-cmd-buff";
            [obj->cmdsBuff retain];
            {
                ScnGpuBuffer_set(&obj->bPropsScns.ref, pBuffPropsScns);
                obj->bPropsScns.obj = bPropsScns;
            }
            {
                ScnGpuBuffer_set(&obj->bPropsMdls.ref, pBuffPropsMdls);
                obj->bPropsMdls.obj = bPropsMdls;
            }
            ScnApiMetalRenderJobState_reset(&obj->state);
            //
            r = ScnTRUE;
        }
    }
    //
    return r;
    
}

ScnBOOL ScnApiMetalRenderJob_buildAddCmds(void* data, const struct STScnRenderCmd* const cmds, const ScnUI32 cmdsSz){
    ScnBOOL r = ScnFALSE;
    //
    STScnApiMetalRenderJob* obj = (STScnApiMetalRenderJob*)data;
    //
    if(obj->cmdsBuff == nil){
        return ScnFALSE;
    } else if(obj->bPropsScns.obj == NULL || obj->bPropsScns.obj->buff == nil){
        return ScnFALSE;
    } else if(obj->bPropsMdls.obj == NULL || obj->bPropsMdls.obj->buff == nil){
        return ScnFALSE;
    }
    //
    STScnApiMetalRenderJobState* state = &obj->state;
    const STScnRenderCmd* c = cmds;
    const STScnRenderCmd* cAfterEnd = cmds + cmdsSz;
    //
    r = ScnTRUE;
    //
    while(r && c < cAfterEnd){
        switch (c->cmdId) {
            case ENScnRenderCmd_None:
                //nop
                break;
                //framebuffers
            case ENScnRenderCmd_ActivateFramebuff:
                if(!ScnFramebuff_isNull(c->activateFramebuff.ref)){
                    ScnGpuFramebuffRef gpuFb = ScnFramebuff_getCurrentRenderSlot(c->activateFramebuff.ref);
                    if(ScnGpuFramebuff_isNull(gpuFb)){
                        SCN_PRINTF_ERROR("ENScnRenderCmd_ActivateFramebuff::ScnGpuFramebuff_isNull.\n");
                        r = ScnFALSE;
                    } else {
                        state->fb = (STScnApiMetalFramebuffView*)ScnGpuFramebuff_getApiItfParam(gpuFb);
                        if(state->fb == NULL || state->fb->mtkView == nil || state->fb->rndrShaders.states[state->fb->cur.verts.type] == nil){
                            SCN_PRINTF_ERROR("ENScnRenderCmd_ActivateFramebuff::fb == NULL || fb->mtkView == nil || fb->renderState == nil.\n");
                            r = ScnFALSE;
                            break;
                        }
                        //ToDo: define how to reset state
                        //ScnMemory_setZeroSt(fb->cur);
                        //
                        MTLRenderPassDescriptor* rndrDesc = state->fb->mtkView.currentRenderPassDescriptor;
                        SCN_ASSERT(obj->cmdsBuff != nil)
                        if(rndrDesc == nil){
                            SCN_PRINTF_ERROR("ENScnRenderCmd_ActivateFramebuff::renderPassDescriptor == nil || commandBuffer == nil.\n");
                            r = ScnFALSE;
                        } else {
                            id <MTLRenderCommandEncoder> rndrEnc = [obj->cmdsBuff renderCommandEncoderWithDescriptor:rndrDesc];
                            if(rndrEnc == nil){
                                SCN_PRINTF_ERROR("ENScnRenderCmd_ActivateFramebuff::renderCommandEncoderWithDescriptor failed.\n");
                                r = ScnFALSE;
                            } else {
                                if(state->rndrDesc != nil){ [state->rndrDesc release]; state->rndrDesc = nil; }
                                if(state->rndrEnc != nil){ [state->rndrEnc release]; state->rndrEnc = nil; }
                                //
                                state->rndrDesc = rndrDesc; [rndrDesc retain];
                                state->rndrEnc = rndrEnc; [rndrEnc retain];
                                state->rndrEnc.label = @"ixtli-render-cmd-enc";
                                //
                                [state->rndrEnc setRenderPipelineState:state->fb->rndrShaders.states[state->fb->cur.verts.type]];
                                //apply viewport
                                {
                                    MTLViewport viewPort;
                                    viewPort.originX = (double)c->activateFramebuff.viewport.x;
                                    viewPort.originY = (double)c->activateFramebuff.viewport.y;
                                    viewPort.width = (double)c->activateFramebuff.viewport.width;
                                    viewPort.height = (double)c->activateFramebuff.viewport.height;
                                    viewPort.znear = 0.0;
                                    viewPort.zfar = 1.0;
                                    [state->rndrEnc setViewport:viewPort];
                                    SCN_PRINTF_VERB("setViewport(%u, %u)-(+%u, +%u).\n", fb->viewport.x, fb->viewport.y, fb->viewport.width, fb->viewport.height);
                                }
                                //fb props
                                [state->rndrEnc setVertexBuffer:obj->bPropsScns.obj->buff offset:c->activateFramebuff.offset atIndex:0];
                                SCN_PRINTF_VERB("setVertexBuffer(idx0, %u offset).\n", c->activateFramebuff.offset);
                                //mdl props
                                [state->rndrEnc setVertexBuffer:obj->bPropsMdls.obj->buff offset:0 atIndex:1];
                                SCN_PRINTF_VERB("setVertexBuffer(idx1, 0 offset).\n");
                            }
                        }
                    }
                }
                break;
            case ENScnRenderCmd_SetFramebuffProps:
                if(state->rndrEnc != nil){
                    //apply viewport
                    {
                        MTLViewport viewPort;
                        viewPort.originX = (double)c->setFramebuffProps.viewport.x;
                        viewPort.originY = (double)c->setFramebuffProps.viewport.y;
                        viewPort.width = (double)c->setFramebuffProps.viewport.width;
                        viewPort.height = (double)c->setFramebuffProps.viewport.height;
                        viewPort.znear = 0.0;
                        viewPort.zfar = 1.0;
                        [state->rndrEnc setViewport:viewPort];
                        SCN_PRINTF_VERB("setViewport(%u, %u)-(+%u, +%u).\n", fb->viewport.x, fb->viewport.y, fb->viewport.width, fb->viewport.height);
                    }
                    //fb props
                    [state->rndrEnc setVertexBufferOffset:c->setFramebuffProps.offset atIndex:0];
                }
                break;
                //models
            case ENScnRenderCmd_SetTransformOffset: //sets the positions of the 'STScnGpuModelProps2d' to be applied for the drawing cmds
                if(state->rndrEnc == nil){
                    SCN_PRINTF_ERROR("ENScnRenderCmd_SetTransformOffset::rndrEnc is nil.\n");
                    r = ScnFALSE;
                } else {
                    [state->rndrEnc setVertexBufferOffset:c->setTransformOffset.offset atIndex:1];
                    SCN_PRINTF_VERB("setVertexBufferOffset(idx1, %u offset).\n", c->setTransformOffset.offset);
                }
                break;
            case ENScnRenderCmd_SetVertexBuff:  //activates the vertex buffer
                if(ScnVertexbuff_isNull(c->setVertexBuff.ref)){
                    SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::ScnVertexbuff_isNull.\n");
                    [state->rndrEnc setVertexBuffer:nil offset:0 atIndex:2];
                } else {
                    ScnGpuVertexbuffRef vbuffRef = ScnVertexbuff_getCurrentRenderSlot(c->setVertexBuff.ref);
                    if(ScnGpuVertexbuff_isNull(vbuffRef)){
                        SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::ScnGpuVertexbuff_isNull.\n");
                        r = ScnFALSE;
                    } else {
                        STScnApiMetalVertexbuff* vbuff = (STScnApiMetalVertexbuff*)ScnGpuVertexBuff_getApiItfParam(vbuffRef);
                        if(vbuff == NULL || ScnGpuBuffer_isNull(vbuff->vBuff)){
                            SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::ScnGpuBuffer_isNull.\n");
                            r = ScnFALSE;
                        } else {
                            STScnApiMetalBuffer* buff = (STScnApiMetalBuffer*)ScnGpuBuffer_getApiItfParam(vbuff->vBuff);
                            STScnApiMetalBuffer* idxs = ScnGpuBuffer_isNull(vbuff->idxBuff) ? NULL : (STScnApiMetalBuffer*)ScnGpuBuffer_getApiItfParam(vbuff->idxBuff);
                            if(buff == NULL){
                                SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::buff == NULL.\n");
                                r = ScnFALSE;
                            } else {
                                const ENScnVertexType vertexType = STScnGpuVertexbuffCfg_2_ENScnVertexType(&vbuff->cfg);
                                if(state->fb->rndrShaders.states[state->fb->cur.verts.type] == nil){
                                    SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::fb->rndrShaders.states[fb->curVertexType] == nil.\n");
                                    r = ScnFALSE;
                                } else {
                                    state->fb->cur.verts.type  = vertexType;
                                    state->fb->cur.verts.buff  = buff;
                                    state->fb->cur.verts.idxs  = idxs;
                                    //MTLRenderStageVertex   = (1UL << 0),
                                    //MTLRenderStageFragment = (1UL << 1),
                                    //if(c->setVertexBuff.isFirstUse){
                                    //    [state->rndrEnc useResource:buff->buff usage:MTLResourceUsageRead stages:MTLRenderStageVertex];
                                    //}
                                    [state->rndrEnc setRenderPipelineState:state->fb->rndrShaders.states[state->fb->cur.verts.type]];
                                    [state->rndrEnc setVertexBuffer:buff->buff offset:0 atIndex:2];
                                    SCN_PRINTF_VERB("setVertexBuffer(idx2, 0 offset).\n");
                                }
                            }
                        }
                    }
                }
                break;
           case ENScnRenderCmd_SetTexture:     //activates the texture in a specific slot-index
                if(ScnTexture_isNull(c->setTexture.ref)){
                    [state->rndrEnc setFragmentTexture:nil atIndex:c->setTexture.index];
                    [state->rndrEnc setFragmentSamplerState:nil atIndex:c->setTexture.index];
                } else {
                    ScnGpuTextureRef texRef = ScnTexture_getCurrentRenderSlot(c->setTexture.ref);
                    if(ScnGpuTexture_isNull(texRef)){
                        SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::ScnGpuTexture_isNull.\n");
                        r = ScnFALSE;
                    } else {
                        STScnApiMetalTexture* tex = (STScnApiMetalTexture*)ScnGpuTexture_getApiItfParam(texRef);
                        if(tex == NULL || tex->tex == nil || ScnGpuSampler_isNull(tex->sampler)){
                            SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::tex->tex is NULL.\n");
                            r = ScnFALSE;
                        } else {
                            STScnApiMetalSampler* smplr = (STScnApiMetalSampler*)ScnGpuSampler_getApiItfParam(tex->sampler);
                            if(smplr->smplr == nil){
                                SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::smplr->smplr is NULL.\n");
                                r = ScnFALSE;
                            } else {
                                //if(c->setTexture.isFirstUse){
                                //    [state->rndrEnc useResource:tex->tex usage:MTLResourceUsageRead stages:MTLRenderStageFragment];
                                //}
                                [state->rndrEnc setFragmentTexture:tex->tex atIndex:c->setTexture.index];
                                [state->rndrEnc setFragmentSamplerState:smplr->smplr atIndex:c->setTexture.index];
                            }
                        }
                    }
                }
                break;
                //modes
                //case ENScnRenderCmd_MaskModePush:   //pushes drawing-mask mode, where only the alpha value is affected
                //    break;
                //case ENScnRenderCmd_MaskModePop:    //pop
                //    break;
                //drawing
            case ENScnRenderCmd_DrawVerts:      //draws something using the vertices
                switch (c->drawVerts.shape) {
                    case ENScnRenderShape_Compute:
                        //nop
                        break;
                        //
                    case ENScnRenderShape_Texture:     //same as 'ENScnRenderShape_TriangStrip' with possible bitblit-optimization if matrix has no rotation.
                        if(state->rndrEnc == NULL){
                            //rintf("ERROR, ENScnRenderShape_Texture::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:c->drawVerts.iFirst vertexCount:c->drawVerts.count];
                            SCN_PRINTF_VERB("drawPrimitives(MTLPrimitiveTypeTriangleStrip: %u, +%u).\n", c->drawVerts.iFirst, c->drawVerts.count);
                        }
                        break;
                    case ENScnRenderShape_TriangStrip: //triangles-strip, most common shape
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_TriangStrip::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:c->drawVerts.iFirst vertexCount:c->drawVerts.count];
                            SCN_PRINTF_VERB("drawPrimitives(MTLPrimitiveTypeTriangleStrip: %u, +%u).\n", c->drawVerts.iFirst, c->drawVerts.count);
                        }
                        break;
                    //case ENScnRenderShape_TriangFan:   //triangles-fan
                    //    break;
                        //
                    case ENScnRenderShape_LineStrip:   //lines-strip
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_LineStrip::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawPrimitives:MTLPrimitiveTypeLineStrip vertexStart:c->drawVerts.iFirst vertexCount:c->drawVerts.count];
                            SCN_PRINTF_VERB("drawPrimitives(MTLPrimitiveTypeLineStrip: %u, +%u).\n", c->drawVerts.iFirst, c->drawVerts.count);
                        }
                        break;
                    //case ENScnRenderShape_LineLoop:    //lines-loop
                    //    break;
                    case ENScnRenderShape_Lines:       //lines
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_Lines::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawPrimitives:MTLPrimitiveTypeLine vertexStart:c->drawVerts.iFirst vertexCount:c->drawVerts.count];
                            SCN_PRINTF_VERB("drawPrimitives(MTLPrimitiveTypeLine: %u, +%u).\n", c->drawVerts.iFirst, c->drawVerts.count);
                        }
                        break;
                    case ENScnRenderShape_Points:      //points
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_Points::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawPrimitives:MTLPrimitiveTypePoint vertexStart:c->drawVerts.iFirst vertexCount:c->drawVerts.count];
                            SCN_PRINTF_VERB("drawPrimitives(MTLPrimitiveTypePoint: %u, +%u).\n", c->drawVerts.iFirst, c->drawVerts.count);
                        }
                        break;
                    default:
                        SCN_ASSERT(ScnFALSE); //missing implementation
                        break;
                }
                break;
            case ENScnRenderCmd_DrawIndexes:    //draws something using the vertices indexes
                if(state->fb == NULL || state->fb->cur.verts.idxs == NULL || state->fb->cur.verts.idxs->buff == nil){
                    SCN_PRINTF_ERROR("ENScnRenderCmd_DrawIndexes without active framebuffer or index-buffer.\n");
                    r = ScnFALSE;
                    break;
                }
                switch (c->drawIndexes.shape) {
                    case ENScnRenderShape_Compute:
                        //nop
                        break;
                        //
                    case ENScnRenderShape_Texture:     //same as 'ENScnRenderShape_TriangStrip' with possible bitblit-optimization if matrix has no rotation.
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_Texture::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawIndexedPrimitives:MTLPrimitiveTypeTriangleStrip indexCount:c->drawIndexes.count indexType:MTLIndexTypeUInt32 indexBuffer:state->fb->cur.verts.idxs->buff indexBufferOffset:c->drawIndexes.iFirst * 4];
                            SCN_PRINTF_VERB("drawIndexedPrimitives(MTLPrimitiveTypeTriangleStrip: %u, +%u).\n", c->drawIndexes.iFirst, c->drawIndexes.count);
                        }
                        break;
                    case ENScnRenderShape_TriangStrip: //triangles-strip, most common shape
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_TriangStrip::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawIndexedPrimitives:MTLPrimitiveTypeTriangleStrip indexCount:c->drawIndexes.count indexType:MTLIndexTypeUInt32 indexBuffer:state->fb->cur.verts.idxs->buff indexBufferOffset:c->drawIndexes.iFirst * 4];
                            SCN_PRINTF_VERB("drawIndexedPrimitives(MTLPrimitiveTypeTriangleStrip: %u, +%u).\n", c->drawIndexes.iFirst, c->drawIndexes.count);
                        }
                        break;
                    //case ENScnRenderShape_TriangFan:   //triangles-fan
                    //    break;
                        //
                    case ENScnRenderShape_LineStrip:   //lines-strip
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_LineStrip::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawIndexedPrimitives:MTLPrimitiveTypeLineStrip indexCount:c->drawIndexes.count indexType:MTLIndexTypeUInt32 indexBuffer:state->fb->cur.verts.idxs->buff indexBufferOffset:c->drawIndexes.iFirst * 4];
                            SCN_PRINTF_VERB("drawIndexedPrimitives(MTLPrimitiveTypeLineStrip: %u, +%u).\n", c->drawIndexes.iFirst, c->drawIndexes.count);
                        }
                        break;
                    //case ENScnRenderShape_LineLoop:    //lines-loop
                    //    break;
                    case ENScnRenderShape_Lines:       //lines
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_Lines::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawIndexedPrimitives:MTLPrimitiveTypeLine indexCount:c->drawIndexes.count indexType:MTLIndexTypeUInt32 indexBuffer:state->fb->cur.verts.idxs->buff indexBufferOffset:c->drawIndexes.iFirst * 4];
                            SCN_PRINTF_VERB("drawIndexedPrimitives(MTLPrimitiveTypeLine: %u, +%u).\n", c->drawIndexes.iFirst, c->drawIndexes.count);
                        }
                        break;
                    case ENScnRenderShape_Points:      //points
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_Points::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawIndexedPrimitives:MTLPrimitiveTypePoint indexCount:c->drawIndexes.count indexType:MTLIndexTypeUInt32 indexBuffer:state->fb->cur.verts.idxs->buff indexBufferOffset:c->drawIndexes.iFirst * 4];
                            SCN_PRINTF_VERB("drawIndexedPrimitives(MTLPrimitiveTypePoint: %u, +%u).\n", c->drawIndexes.iFirst, c->drawIndexes.count);
                        }
                        break;
                    default:
                        SCN_ASSERT(ScnFALSE); //missing implementation
                        break;
                }
                break;
            default:
                SCN_ASSERT(ScnFALSE) //missing implementation
                break;
        }
        ++c;
    }
    return r;
}

ScnBOOL ScnApiMetalRenderJob_buildEndAndEnqueue(void* data){
    ScnBOOL r = ScnFALSE;
    STScnApiMetalRenderJob* obj = (STScnApiMetalRenderJob*)data;
    STScnApiMetalRenderJobState* state = &obj->state;
    if(state->rndrEnc == nil || obj->cmdsBuff == nil || state->fb == NULL){
        return ScnFALSE;
    }
    //
    r = ScnTRUE;
    //finalize
    if(state->rndrEnc != nil){
        [state->rndrEnc endEncoding];
        SCN_PRINTF_VERB("endEncoding.\n");
    }
    if(obj->cmdsBuff != nil){
        if(state->fb != NULL && state->fb->mtkView != NULL){
            [obj->cmdsBuff presentDrawable:state->fb->mtkView.currentDrawable];
            SCN_PRINTF_VERB("presentDrawable.\n");
        }
        [obj->cmdsBuff commit];
        SCN_PRINTF_VERB("commit.\n");
    }
    return r;
}
