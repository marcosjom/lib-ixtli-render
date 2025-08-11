//
//  ScnRender.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/ScnRender.h"
#include "ixrender/scene/ScnRenderJob.h"
#include "ixrender/core/ScnArray.h"
#include "ixrender/core/ScnArraySorted.h"
#include "ixrender/gpu/ScnGpuDataType.h"
#include "ixrender/gpu/ScnGpuFramebuffProps.h"
#include "ixrender/gpu/ScnGpuModelProps2d.h"
#include "ixrender/gpu/ScnGpuRenderJob.h"
#include "ixrender/scene/ScnRenderCmds.h"

//STScnRenderOpq

typedef struct STScnRenderSlot {
    ScnGpuBufferRef     bPropsScns;     //buffer with scenes properties (viewport, ortho-box, ...)
    ScnGpuBufferRef     bPropsMdls;     //buffer with models properties (color, matrices, ...)
    STScnRenderCmds     cmds;           //cmds buffer
    ScnGpuRenderJobRef  gpuJob;         //
} STScnRenderSlot;

void    ScnRenderSlot_init(ScnContextRef ctx, STScnRenderSlot* obj);
void    ScnRenderSlot_destroy(STScnRenderSlot* obj);

//STScnRenderOpq

typedef struct STScnRenderOpq {
    ScnContextRef       ctx;
    ScnMutexRef         mutex;
    //api
    struct {
        STScnApiItf     itf;
        void*           itfParam;
    } api;
    ScnGpuDeviceRef     gpuDev;
    STScnGpuDeviceDesc  gpuDevDesc;
    //
    ScnVertexbuffsRef   vbuffs;         //buffers with vertices and indices
    //slots
    struct {
        STScnRenderSlot* arr;
        ScnUI32         use;
    } slots;
} STScnRenderOpq;

//

ScnSI32 ScnRender_getOpqSz(void){
    return (ScnSI32)sizeof(STScnRenderOpq);
}

void ScnRender_initZeroedOpq(ScnContextRef ctx, void* obj) {
    STScnRenderOpq* opq = (STScnRenderOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex = ScnContext_allocMutex(opq->ctx);
    //api
    {
        //
    }
    //gpuDev
    {
        //
    }
    //slots
    {
        //
    }
}

void ScnRender_destroyOpq(void* obj){
    STScnRenderOpq* opq = (STScnRenderOpq*)obj;
    //slots
    if(opq->slots.arr != NULL){
        //ToDo: wait for any active job completion
        STScnRenderSlot* s = opq->slots.arr;
        const STScnRenderSlot* sAfterEnd = s + opq->slots.use;
        while(s < sAfterEnd){
            ScnRenderSlot_destroy(s);
            //
            ++s;
        }
        ScnContext_mfree(opq->ctx, opq->slots.arr);
        opq->slots.arr = NULL;
        opq->slots.use = 0;
    }
    //
    ScnVertexbuffs_releaseAndNull(&opq->vbuffs);
    ScnGpuDevice_releaseAndNull(&opq->gpuDev);
    //api
    {
        ScnMemory_setZeroSt(opq->api.itf);
        opq->api.itfParam = NULL;
    }
    //
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNull(&opq->ctx);
}

//prepare

ScnBufferRef      ScnRender_allocDynamicBuffLockedOpq_(ScnContextRef ctx, ScnGpuDeviceRef gpuDev, const ScnUI32 memBlockAlign, const ScnUI32 offsetsAlignment, const ScnUI32 itmSz, const ScnUI32 itmsPerBlock, const ScnUI32 ammRenderSlots);
ScnVertexbuffsRef ScnRender_allocVertexbuffsLockedOpq_(ScnContextRef ctx, ScnGpuDeviceRef gpuDev, const ScnUI32 memBlockAlign, const ScnUI32 ammRenderSlots);

ScnBOOL ScnRender_prepare(ScnRenderRef ref, const STScnApiItf* itf, void* itfParam){
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

ScnBOOL ScnRender_openDevice(ScnRenderRef ref, const STScnGpuDeviceCfg* cfg, const ScnUI32 ammRenderSlots){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(ScnGpuDevice_isNull(opq->gpuDev) && opq->api.itf.allocDevice != NULL && opq->slots.use == 0 && ammRenderSlots > 0){
        ScnGpuDeviceRef gpuDev = (*opq->api.itf.allocDevice)(opq->ctx, cfg);
        if(ScnGpuDevice_isNull(gpuDev)){
            //error
        } else {
            const STScnGpuDeviceDesc devDesc = ScnGpuDevice_getDesc(gpuDev);
            ScnUI32 slotsUse = 0;
            STScnRenderSlot* slots = (STScnRenderSlot*)ScnContext_malloc(opq->ctx, sizeof(STScnRenderSlot) * ammRenderSlots, "ScnRender_openDevice::slots");
            if(slots == NULL){
                //error
            } else {
                //init slots
                {
                    STScnRenderSlot* s = slots;
                    const STScnRenderSlot* sAfterEnd = s + ammRenderSlots;
                    while(s < sAfterEnd){
                        ScnRenderSlot_init(opq->ctx, s);
                        ++slotsUse;
                        //
                        ++s;
                    }
                }
                //initial buffers
                if(slotsUse == ammRenderSlots){
                    ScnVertexbuffsRef vbuffs = ScnRender_allocVertexbuffsLockedOpq_(opq->ctx, gpuDev, devDesc.memBlockAlign, ammRenderSlots);
                    if(ScnVertexbuffs_isNull(vbuffs)){
                        printf("ERROR, ScnRender_allocVertexbuffsLockedOpq_(vbuffs) failed.\n");
                    } else {
                        opq->gpuDevDesc = devDesc;
                        ScnGpuDevice_set(&opq->gpuDev, gpuDev);
                        ScnVertexbuffs_set(&opq->vbuffs, vbuffs);
                        opq->slots.arr  = slots;    slots = NULL; //consume
                        opq->slots.use  = slotsUse; slotsUse = 0;
                        r = ScnTRUE;
                    }
                    ScnVertexbuffs_releaseAndNull(&vbuffs);
                }
            }
            //release (if not consumed)
            if(slots != NULL){
                STScnRenderSlot* s = slots;
                const STScnRenderSlot* sAfterEnd = s + slotsUse;
                while(s < sAfterEnd){
                    ScnRenderSlot_destroy(s);
                    //
                    ++s;
                }
                ScnContext_mfree(opq->ctx, slots);
                slots = NULL;
                slotsUse = 0;
            }
            //
            ScnGpuDevice_releaseAndNull(&gpuDev);
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRender_hasOpenDevice(ScnRenderRef ref){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return !ScnGpuDevice_isNull(opq->gpuDev);
}

void* ScnRender_getApiDevice(ScnRenderRef ref){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return !ScnGpuDevice_isNull(opq->gpuDev) ? ScnGpuDevice_getApiDevice(opq->gpuDev) : NULL;
}

STScnGpuDeviceDesc ScnRender_getDeviceDesc(ScnRenderRef ref){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return !ScnGpuDevice_isNull(opq->gpuDev) ? ScnGpuDevice_getDesc(opq->gpuDev) : (STScnGpuDeviceDesc)STScnGpuDeviceDesc_Zero;
}

ScnModel2dRef ScnRender_allocModel(ScnRenderRef ref){
    ScnModel2dRef r = ScnObjRef_Zero;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnVertexbuffs_isNull(opq->vbuffs)){
        r = ScnModel2d_alloc(opq->ctx);
        if(ScnModel2d_isNull(r)){
            printf("ERROR, ScnRender_allocModel::ScnModel2d_alloc failed.\n");
        } else if(!ScnModel2d_setVertexBuffs(r, opq->vbuffs)){
            printf("ERROR, ScnRender_allocModel::ScnModel2d_setVertexBuffs failed.\n");
            ScnModel2d_releaseAndNull(&r);
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//framebuffer

ScnFramebuffRef ScnRender_allocFramebuff(ScnRenderRef ref){
    ScnFramebuffRef r = ScnObjRef_Zero;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnGpuDevice_isNull(opq->gpuDev)){
        ScnFramebuffRef fb = ScnFramebuff_alloc(opq->ctx);
        if(ScnFramebuff_isNull(fb)){
            printf("ERROR, ScnRender_allocFramebuff::ScnFramebuff_alloc failed.\n");
        } else if(!ScnFramebuff_prepare(fb, opq->gpuDev)){
            printf("ERROR, ScnRender_allocFramebuff::ScnModel2d_setVertexBuffs failed.\n");
        } else {
            ScnFramebuff_set(&r, fb);
        }
        ScnFramebuff_releaseAndNull(&fb);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//texture

ScnTextureRef ScnRender_allocTexture(ScnRenderRef ref, const ENScnResourceMode mode, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const optSrcProps, const void* optSrcData){
    ScnTextureRef r = ScnObjRef_Zero;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnGpuDevice_isNull(opq->gpuDev) && opq->slots.use > 0){
        ScnTextureRef fb = ScnTexture_alloc(opq->ctx);
        if(ScnTexture_isNull(fb)){
            printf("ERROR, ScnRender_allocTexture::ScnTexture_alloc failed.\n");
        } else if(!ScnTexture_prepare(fb, opq->gpuDev, mode, (mode == ENScnResourceMode_Dynamic ? opq->slots.use : 1), cfg, optSrcProps, optSrcData)){
            printf("ERROR, ScnRender_allocTexture::ScnTexture_prepare failed.\n");
        } else {
            ScnTexture_set(&r, fb);
        }
        ScnTexture_releaseAndNull(&fb);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//sampler

ScnGpuSamplerRef ScnRender_allocSampler(ScnRenderRef ref, const STScnGpuSamplerCfg* const cfg){
    ScnGpuSamplerRef r = ScnObjRef_Zero;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnGpuDevice_isNull(opq->gpuDev)){
        ScnGpuSamplerRef s = ScnGpuDevice_allocSampler(opq->gpuDev, cfg);
        if(ScnGpuSampler_isNull(s)){
            printf("ERROR, ScnRender_allocTexture::ScnGpuDevice_allocSampler failed.\n");
            ScnGpuSampler_set(&r, s);
        }
        ScnGpuSampler_releaseAndNull(&s);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//renderJob

STScnRenderSlot* ScnRender_getAvailRenderSlotLockedOpq_(STScnRenderOpq* opq){
    STScnRenderSlot* s = opq->slots.arr;
    const STScnRenderSlot* sAfterEnd = s + opq->slots.use;
    while(s < sAfterEnd){
        ENScnGpuRenderJobState state;
        if(
           ScnGpuRenderJob_isNull(s->gpuJob)
           || ENScnGpuRenderJobState_Completed == (state = ScnGpuRenderJob_getState(s->gpuJob))
           || ENScnGpuRenderJobState_Error == state
           )
        {
            return s;
        }
        //
        ++s;
    }
    return NULL;
}

ScnRenderJobRef ScnRender_allocRenderJob(ScnRenderRef ref){
    ScnRenderJobRef r = ScnRenderJobRef_Zero;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnGpuDevice_isNull(opq->gpuDev) && opq->slots.arr != NULL){
        //Find an available render slot
        STScnRenderSlot* sAvail = ScnRender_getAvailRenderSlotLockedOpq_(opq);
        if(sAvail != NULL){
            if(!sAvail->cmds.isPrepared && !ScnRenderCmds_prepare(&sAvail->cmds, &opq->gpuDevDesc)){
                printf("ERROR, ScnRender_allocRenderJob::ScnRenderCmds_prepare failed.\n");
            } else {
                ScnGpuRenderJobRef gpuJob = ScnGpuRenderJobRef_Zero;
                ScnRenderCmds_reset(&sAvail->cmds);
                {
                    //Create a RenderJob re-using previously cmds-buffer and nullifying its previous GpuJob.
                    ScnRenderJobRef job = ScnRenderJob_alloc(opq->ctx);
                    if(!ScnRenderJob_isNull(job) && ScnRenderJob_swapCmds(job, &sAvail->cmds, &gpuJob)){
                        ScnRenderJob_set(&r, job);
                    }
                    ScnRenderJob_releaseAndNull(&job);
                }
                ScnGpuRenderJob_releaseAndNull(&gpuJob);
            }
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnRender_enqueue(ScnRenderRef ref, ScnRenderJobRef job){
    ScnBOOL r = ScnFALSE;
    if(ScnRenderJob_isNull(job)){
        //invalida params
        return ScnFALSE;
    }
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(!ScnGpuDevice_isNull(opq->gpuDev) && opq->slots.arr != NULL){
        //Find an available render slot
        STScnRenderSlot* sAvail = ScnRender_getAvailRenderSlotLockedOpq_(opq);
        if(sAvail != NULL){
            ScnGpuRenderJobRef gpuJob = ScnGpuDevice_allocRenderJob(opq->gpuDev);
            if(ScnGpuRenderJob_isNull(gpuJob)){
                printf("ERROR, ScnRender_enqueue::ScnGpuDevice_allocRenderJob failed.\n");
            } else {
                ScnGpuRenderJobRef gpuJobPrev = gpuJob;
                ScnGpuRenderJob_retain(gpuJobPrev);
                if(!ScnRenderJob_swapCmds(job, &sAvail->cmds, &gpuJobPrev)){
                    printf("ERROR, ScnRender_enqueue::ScnRenderJob_swapCmds failed.\n");
                    ScnGpuRenderJob_release(&gpuJobPrev);
                } else if(!sAvail->cmds.isPrepared || ScnMemElastic_isNull(sAvail->cmds.mPropsScns) || ScnMemElastic_isNull(sAvail->cmds.mPropsMdls)){
                    printf("ERROR, ScnRender_enqueue::cmds were not previosuly prepared.\n");
                } else {
                    r = ScnTRUE;
                    //printf("ScnRenderJob_end::%u objs, %u cmds.\n", opq->cmds.objs.use, opq->cmds.cmds.use);
                    //sync buffers
                    if(r && ScnMemElastic_hasPtrs(sAvail->cmds.mPropsScns)){
                        if(ScnGpuBuffer_isNull(sAvail->bPropsScns)){
                            //create new buffer
                            sAvail->bPropsScns = ScnGpuDevice_allocBuffer(opq->gpuDev, sAvail->cmds.mPropsScns);
                            if(ScnGpuBuffer_isNull(sAvail->bPropsScns)){
                                printf("ERROR, ScnRender_enqueue::ScnGpuBuffer_sync(bPropsScns) failed.\n");
                                r = ScnFALSE;
                            }
                        } else {
                            //sync existing buffer
                            const STScnGpuBufferChanges chngs = STScnGpuBufferChanges_All;
                            if(!ScnGpuBuffer_sync(sAvail->bPropsScns, sAvail->cmds.mPropsScns, &chngs)){
                                printf("ERROR, ScnRender_enqueue::ScnGpuBuffer_sync(bPropsScns) failed.\n");
                            }
                        }
                    }
                    if(r && ScnMemElastic_hasPtrs(sAvail->cmds.mPropsMdls)){
                        if(ScnGpuBuffer_isNull(sAvail->bPropsMdls)){
                            //create new buffer
                            sAvail->bPropsMdls = ScnGpuDevice_allocBuffer(opq->gpuDev, sAvail->cmds.mPropsMdls);
                            if(ScnGpuBuffer_isNull(sAvail->bPropsMdls)){
                                printf("ERROR, ScnRender_enqueue::ScnGpuBuffer_sync(bPropsMdls) failed.\n");
                                r = ScnFALSE;
                            }
                        } else {
                            //sync existing buffer
                            const STScnGpuBufferChanges chngs = STScnGpuBufferChanges_All;
                            if(!ScnGpuBuffer_sync(sAvail->bPropsMdls, sAvail->cmds.mPropsMdls, &chngs)){
                                printf("ERROR, ScnRender_enqueue::ScnGpuBuffer_sync(bPropsMdls) failed.\n");
                            }
                        }
                    }
                    //sync used objects' gpu-data
                    if(r){
                        ScnArraySorted_foreach(&sAvail->cmds.objs, STScnRenderJobObj, o,
                            switch(o->type){
                                case ENScnRenderJobObjType_Unknown: break;
                                case ENScnRenderJobObjType_Buff:
                                    if(!ScnBuffer_prepareCurrentRenderSlot(o->buff, NULL)){
                                        printf("ScnRenderJob_end::ScnBuffer_prepareCurrentRenderSlot failed.\n");
                                        r = ScnFALSE;
                                        break;
                                    }
                                    break;
                                case ENScnRenderJobObjType_Framebuff:
                                    if(!ScnFramebuff_prepareCurrentRenderSlot(o->framebuff)){
                                        printf("ScnRenderJob_end::ScnFramebuff_prepareCurrentRenderSlot failed.\n");
                                        r = ScnFALSE;
                                        break;
                                    }
                                    break;
                                case ENScnRenderJobObjType_Vertexbuff:
                                    if(!ScnVertexbuff_prepareCurrentRenderSlot(o->vertexbuff)){
                                        printf("ScnRenderJob_end::ScnVertexbuff_prepareCurrentRenderSlot failed.\n");
                                        r = ScnFALSE;
                                        break;
                                    }
                                    break;
                                case ENScnRenderJobObjType_Texture:
                                    if(!ScnTexture_prepareCurrentRenderSlot(o->texture)){
                                        printf("ScnRenderJob_end::ScnTexture_prepareCurrentRenderSlot failed.\n");
                                        r = ScnFALSE;
                                        break;
                                    }
                                    break;
                                default:
                                    SCN_ASSERT(ScnFALSE) //missing implementation
                                    break;
                            }
                        );
                    }
                    //send render commands to gpu
                    if(r && sAvail->cmds.cmds.use > 0){
                        if(!ScnGpuRenderJob_buildBegin(gpuJob, sAvail->bPropsScns, sAvail->bPropsMdls)){
                            printf("ERROR, ScnRender_enqueue::ScnGpuRenderJob_buildBegin failed.\n");
                            r = ScnFALSE;
                        } else if(!ScnGpuRenderJob_buildAddCmds(gpuJob, sAvail->cmds.cmds.arr, sAvail->cmds.cmds.use)){
                            printf("ERROR, ScnRender_enqueue::ScnGpuRenderJob_buildAddCmds failed.\n");
                            r = ScnFALSE;
                        } else if(!ScnGpuRenderJob_buildEndAndEnqueue(gpuJob)){
                            printf("ERROR, ScnRender_enqueue::ScnGpuRenderJob_buildEndAndEnqueue failed.\n");
                            r = ScnFALSE;
                        }
                    }
                    //move used-objects to their next render slot
                    if(r){
                        ScnArraySorted_foreach(&sAvail->cmds.objs, STScnRenderJobObj, o,
                            switch(o->type){
                                case ENScnRenderJobObjType_Unknown: break;
                                case ENScnRenderJobObjType_Buff:
                                    if(!ScnBuffer_moveToNextRenderSlot(o->buff)){
                                        printf("ScnRenderJob_end::ScnBuffer_moveToNextRenderSlot failed.\n");
                                        r = ScnFALSE;
                                        break;
                                    }
                                    break;
                                case ENScnRenderJobObjType_Framebuff:
                                    if(!ScnFramebuff_moveToNextRenderSlot(o->framebuff)){
                                        printf("ScnRenderJob_end::ScnFramebuff_moveToNextRenderSlot failed.\n");
                                        r = ScnFALSE;
                                        break;
                                    }
                                    break;
                                case ENScnRenderJobObjType_Vertexbuff:
                                    if(!ScnVertexbuff_moveToNextRenderSlot(o->vertexbuff)){
                                        printf("ScnRenderJob_end::ScnVertexbuff_moveToNextRenderSlot failed.\n");
                                        r = ScnFALSE;
                                        break;
                                    }
                                    break;
                                case ENScnRenderJobObjType_Texture:
                                    if(!ScnTexture_moveToNextRenderSlot(o->texture)){
                                        printf("ScnRenderJob_end::ScnTexture_moveToNextRenderSlot failed.\n");
                                        r = ScnFALSE;
                                        break;
                                    }
                                    break;
                                default:
                                    SCN_ASSERT(ScnFALSE) //missing implementation
                                    break;
                            }
                        );
                    }
                }
                ScnGpuRenderJob_releaseAndNull(&gpuJobPrev);
            }
            ScnGpuRenderJob_releaseAndNull(&gpuJob);
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//Vertices

ScnVertexbuffsRef ScnRender_getDefaultVertexbuffs(ScnRenderRef ref){
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->vbuffs;
}

ScnVertexbuffsRef ScnRender_allocVertexbuffs(ScnRenderRef ref){
    ScnVertexbuffsRef r = ScnObjRef_Zero;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(opq->slots.use > 0){
        r = ScnRender_allocVertexbuffsLockedOpq_(opq->ctx, opq->gpuDev, opq->gpuDevDesc.memBlockAlign, opq->slots.use);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBufferRef ScnRender_allocDynamicBuffLockedOpq_(ScnContextRef ctx, ScnGpuDeviceRef gpuDev, const ScnUI32 memBlockAlign, const ScnUI32 offsetsAlignment, const ScnUI32 itmSz, const ScnUI32 itmsPerBlock, const ScnUI32 ammRenderSlots){
    ScnBufferRef r        = ScnObjRef_Zero;
    if(offsetsAlignment <= 0 || memBlockAlign <= 0){
        //invalid arguments
        return r;
    }
    STScnGpuBufferCfg cfg   = STScnGpuBufferCfg_Zero;
    cfg.mem.idxsAlign       = (itmSz + offsetsAlignment - 1) / offsetsAlignment * offsetsAlignment;
    cfg.mem.sizeAlign       = memBlockAlign;
    cfg.mem.sizeInitial     = 0;
    //calculate sizePerBlock
    {
        ScnUI32 idxExtra = 0;
        cfg.mem.sizePerBlock = ((itmsPerBlock * cfg.mem.idxsAlign) + cfg.mem.sizeAlign - 1) / cfg.mem.sizeAlign * cfg.mem.sizeAlign;
        idxExtra = cfg.mem.sizePerBlock % cfg.mem.idxsAlign;
        if(idxExtra > 0){
            cfg.mem.sizePerBlock *= (cfg.mem.idxsAlign - idxExtra);
        }
        //SCN_ASSERT(0 == (cfg.mem.sizePerBlock % cfg.mem.idxsAlign))
        //SCN_ASSERT(0 == (cfg.mem.sizePerBlock % cfg.mem.sizeAlign))
    }
    r = ScnBuffer_alloc(ctx);
    if(ScnBuffer_isNull(r)){
        //error
        SCN_ASSERT(ScnFALSE)
    } else if(!ScnBuffer_prepare(r, gpuDev, ammRenderSlots, &cfg)){
        //error
        SCN_ASSERT(ScnFALSE)
        ScnBuffer_releaseAndNull(&r);
    }
    return r;
}

ScnVertexbuffsRef ScnRender_allocVertexbuffsLockedOpq_(ScnContextRef ctx, ScnGpuDeviceRef gpuDev, const ScnUI32 memBlockAlign, const ScnUI32 ammRenderSlots){
    ScnVertexbuffsRef rr = ScnObjRef_Zero;
    ScnBOOL r = ScnTRUE;
    ScnBufferRef vBuffs[ENScnVertexType_Count];
    ScnBufferRef iBuffs[ENScnVertexType_Count];
    ScnVertexbuffRef vbs[ENScnVertexType_Count];
    ScnMemory_setZeroSt(vBuffs);
    ScnMemory_setZeroSt(iBuffs);
    ScnMemory_setZeroSt(vbs);
    //initial bufffers
    if(r){
        ScnSI32 i; for(i = 0; i < ENScnVertexType_Count; i++){
            ScnUI32 itmSz = 0, ammPerBock = 0;
            switch(i){
                case ENScnVertexType_2DColor:   ammPerBock = 256;  itmSz = sizeof(STScnVertex2D); break;
                case ENScnVertexType_2DTex:     ammPerBock = 1024; itmSz = sizeof(STScnVertex2DTex); break;
                case ENScnVertexType_2DTex2:    ammPerBock = 256;  itmSz = sizeof(STScnVertex2DTex2); break;
                case ENScnVertexType_2DTex3:    ammPerBock = 256;  itmSz = sizeof(STScnVertex2DTex3); break;
                default: SCN_ASSERT(ScnFALSE); r = ScnFALSE; break; //missing implementation
            }
            //vertex buffer
            if(r){
                vBuffs[i] = ScnRender_allocDynamicBuffLockedOpq_(ctx, gpuDev, memBlockAlign, itmSz, itmSz, ammPerBock, ammRenderSlots);
                if(ScnBuffer_isNull(vBuffs[i])){
                    //error
                    SCN_ASSERT(ScnFALSE)
                    r = ScnFALSE;
                }
            }
            //indices buffer
            if(r){
                itmSz = sizeof(STScnVertexIdx);
                ammPerBock = 2048;
                iBuffs[i] = ScnRender_allocDynamicBuffLockedOpq_(ctx, gpuDev, memBlockAlign, itmSz, itmSz, ammPerBock, ammRenderSlots);
                if(ScnBuffer_isNull(iBuffs[i])){
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
            ScnBufferRef vertexBuff = vBuffs[i];
            ScnBufferRef idxsBuff = iBuffs[i];
            //size
            switch(i){
                case ENScnVertexType_2DColor:   cfg.szPerRecord = sizeof(STScnVertex2D); break;
                case ENScnVertexType_2DTex:     cfg.szPerRecord = sizeof(STScnVertex2DTex); break;
                case ENScnVertexType_2DTex2:    cfg.szPerRecord = sizeof(STScnVertex2DTex2); break;
                case ENScnVertexType_2DTex3:    cfg.szPerRecord = sizeof(STScnVertex2DTex3); break;
                default: SCN_ASSERT(ScnFALSE) r = ScnFALSE; break; //missing implementation
            }
            //SCN_ASSERT((cfg.szPerRecord % 4) == 0)
            //elems
            switch(i){
                case ENScnVertexType_2DTex3:
                    cfg.texCoords[2].amm    = 2;
                    cfg.texCoords[2].type   = ENScnGpuDataType_FLOAT32;
                    cfg.texCoords[2].offset = ScnVertex2DTex3_IDX_tex3_x;
                case ENScnVertexType_2DTex2:
                    cfg.texCoords[1].amm    = 2;
                    cfg.texCoords[1].type   = ENScnGpuDataType_FLOAT32;
                    cfg.texCoords[1].offset = ScnVertex2DTex2_IDX_tex2_x;
                case ENScnVertexType_2DTex:
                    cfg.texCoords[0].amm    = 2;
                    cfg.texCoords[0].type   = ENScnGpuDataType_FLOAT32;
                    cfg.texCoords[0].offset = ScnVertex2DTex_IDX_tex_x;
                default:
                    //color
                    cfg.color.amm           = 4;
                    cfg.color.type          = ENScnGpuDataType_UI8;
                    cfg.color.offset        = ScnVertex2D_IDX_color;
                    //coord
                    cfg.coord.amm           = 2;
                    cfg.coord.type          = ENScnGpuDataType_FLOAT32;
                    cfg.coord.offset        = ScnVertex2D_IDX_x;
                    break;
            }
            if(r){
                vbs[i] = ScnVertexbuff_alloc(ctx);
                if(!ScnVertexbuff_prepare(vbs[i], gpuDev, ammRenderSlots, &cfg, vertexBuff, idxsBuff)){
                    //error
                    SCN_ASSERT(ScnFALSE)
                    r = ScnFALSE;
                }
            }
        }
        //
        if(r){
            ScnVertexbuffsRef vbObj = ScnVertexbuffs_alloc(ctx);
            if(ScnVertexbuffs_prepare(vbObj, vbs, sizeof(vbs) / sizeof(vbs[0]))){
                ScnVertexbuffs_set(&rr, vbObj);
            } else {
                r = ScnFALSE;
            }
            ScnVertexbuffs_releaseAndNull(&vbObj);
        }
    }
    //release
    {
        //vbs
        {
            ScnVertexbuffRef* b = vbs;
            const ScnVertexbuffRef* bAfterEnd = b + (sizeof(vbs) / sizeof(vbs[0]));
            while(b < bAfterEnd){
                ScnVertexbuff_release(b);
                ++b;
            }
        }
        //vBuffs
        {
            ScnBufferRef* b = vBuffs;
            const ScnBufferRef* bAfterEnd = b + (sizeof(vBuffs) / sizeof(vBuffs[0]));
            while(b < bAfterEnd){
                ScnBuffer_release(b);
                ++b;
            }
        }
        //iBuffs
        {
            ScnBufferRef* b = iBuffs;
            const ScnBufferRef* bAfterEnd = b + (sizeof(iBuffs) / sizeof(iBuffs[0]));
            while(b < bAfterEnd){
                ScnBuffer_release(b);
                ++b;
            }
        }
    }
    return rr;
}

//Buffers

ScnBufferRef ScnRender_allocBuffer(ScnRenderRef ref, const STScnGpuBufferCfg* const cfg){ //allocates a new buffer
    ScnBufferRef r = ScnObjRef_Zero;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(cfg != NULL && opq->slots.use > 0){
        ScnBufferRef b = ScnBuffer_alloc(opq->ctx);
        if(!ScnBuffer_prepare(b, opq->gpuDev, opq->slots.use, cfg)){
            //error
            SCN_ASSERT(ScnFALSE)
        } else {
            ScnBuffer_set(&r, b);
        }
        ScnBuffer_releaseAndNull(&b);
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

//STScnRenderOpq

void ScnRenderSlot_init(ScnContextRef ctx, STScnRenderSlot* obj){
    ScnMemory_setZeroSt(*obj);
    ScnRenderCmds_init(ctx, &obj->cmds);
}

void ScnRenderSlot_destroy(STScnRenderSlot* obj){
    ScnRenderCmds_destroy(&obj->cmds);
    ScnGpuRenderJob_releaseAndNull(&obj->gpuJob);
    ScnGpuBuffer_releaseAndNull(&obj->bPropsScns);
    ScnGpuBuffer_releaseAndNull(&obj->bPropsMdls);
}

