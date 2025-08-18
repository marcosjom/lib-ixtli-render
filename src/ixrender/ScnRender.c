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
    STScnRenderCfg      cfg;
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

ScnUI32 ScnRender_getOpqSz(void){
    return (ScnUI32)sizeof(STScnRenderOpq);
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

//

STScnRenderCfg ScnRender_getDefaultCfg(void){
    STScnRenderCfg r;
    ScnMemory_setZeroSt(r);
    //memory
    {
        r.mem.propsScnsPerBlock     = 32;
        r.mem.propsMdlsPerBlock     = 128;
        r.mem.verts.idxsPerBlock    = 2048;
        r.mem.verts.typesPerBlock[ENScnVertexType_2DColor]  = 256;
        r.mem.verts.typesPerBlock[ENScnVertexType_2DTex]    = 1024;
        r.mem.verts.typesPerBlock[ENScnVertexType_2DTex2]   = 256;
        r.mem.verts.typesPerBlock[ENScnVertexType_2DTex3]   = 256;
    }
    return r;
}

//prepare

ScnBufferRef      ScnRender_allocDynamicBuffLockedOpq_(ScnContextRef ctx, ScnGpuDeviceRef gpuDev, const ScnUI32 memBlockAlign, const ScnUI32 offsetsAlignment, const ScnUI32 itmSz, const ScnUI32 itmsPerBlock, const ScnUI32 ammRenderSlots);
ScnVertexbuffsRef ScnRender_allocVertexbuffsLockedOpq_(ScnContextRef ctx, ScnGpuDeviceRef gpuDev, const ScnUI32 memBlockAlign, const ScnUI32 ammRenderSlots, const STScnRenderVertsCfg* cfg);

ScnBOOL ScnRender_prepare(ScnRenderRef ref, const STScnApiItf* itf, void* itfParam, const STScnRenderCfg* optCfg){
    ScnBOOL r = ScnFALSE;
    STScnRenderOpq* opq = (STScnRenderOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    if(itf != NULL){
        opq->api.itf    = *itf;
        opq->api.itfParam = itfParam;
        opq->cfg        = (optCfg == NULL ? ScnRender_getDefaultCfg() : *optCfg);
        //ToDo: validate cfg values here, or allow it to produce error later?
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
            STScnRenderSlot* slots = (STScnRenderSlot*)ScnContext_malloc(opq->ctx, sizeof(STScnRenderSlot) * ammRenderSlots, SCN_DBG_STR("ScnRender_openDevice::slots"));
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
                    ScnVertexbuffsRef vbuffs = ScnRender_allocVertexbuffsLockedOpq_(opq->ctx, gpuDev, devDesc.memBlockAlign, ammRenderSlots, &opq->cfg.mem.verts);
                    if(ScnVertexbuffs_isNull(vbuffs)){
                        SCN_PRINTF_ERROR("ScnRender_allocVertexbuffsLockedOpq_(vbuffs) failed.\n");
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
            SCN_PRINTF_ERROR("ScnRender_allocModel::ScnModel2d_alloc failed.\n");
        } else if(!ScnModel2d_setVertexBuffs(r, opq->vbuffs)){
            SCN_PRINTF_ERROR("ScnRender_allocModel::ScnModel2d_setVertexBuffs failed.\n");
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
            SCN_PRINTF_ERROR("ScnRender_allocFramebuff::ScnFramebuff_alloc failed.\n");
        } else if(!ScnFramebuff_prepare(fb, opq->gpuDev)){
            SCN_PRINTF_ERROR("ScnRender_allocFramebuff::ScnModel2d_setVertexBuffs failed.\n");
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
            SCN_PRINTF_ERROR("ScnRender_allocTexture::ScnTexture_alloc failed.\n");
        } else if(!ScnTexture_prepare(fb, opq->gpuDev, mode, (mode == ENScnResourceMode_Dynamic ? opq->slots.use : 1), cfg, optSrcProps, optSrcData)){
            SCN_PRINTF_ERROR("ScnRender_allocTexture::ScnTexture_prepare failed.\n");
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
            SCN_PRINTF_ERROR("ScnRender_allocTexture::ScnGpuDevice_allocSampler failed.\n");
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
        const ENScnGpuRenderJobState state = (ScnGpuRenderJob_isNull(s->gpuJob) ? ENScnGpuRenderJobState_Unknown : ScnGpuRenderJob_getState(s->gpuJob));
        if(
           ScnGpuRenderJob_isNull(s->gpuJob)
           || state == ENScnGpuRenderJobState_Completed
           || state == ENScnGpuRenderJobState_Error
           )
        {
#           ifdef SCN_ASSERTS_ACTIVATED
            if(ScnGpuRenderJob_isNull(s->gpuJob)){
                SCN_PRINTF_INFO("ScnRender::first job in slot.\n");
            } else if(state == ENScnGpuRenderJobState_Completed){
                //SCN_PRINTF_INFO("ScnRender::previous job explicitely completed.\n");
            } if(ENScnGpuRenderJobState_Error == state){
                SCN_PRINTF_WARNING("ScnRender::previous job ended with error.\n");
            }
#           endif
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
        STScnRenderSlot* slot = ScnRender_getAvailRenderSlotLockedOpq_(opq);
        if(slot != NULL){
            if(!slot->cmds.isPrepared && !ScnRenderCmds_prepare(&slot->cmds, &opq->gpuDevDesc, opq->cfg.mem.propsScnsPerBlock, opq->cfg.mem.propsMdlsPerBlock)){
                SCN_PRINTF_ERROR("ScnRender_allocRenderJob::ScnRenderCmds_prepare failed.\n");
            } else {
                ScnGpuRenderJobRef gpuJob = ScnGpuRenderJobRef_Zero;
                ScnRenderCmds_reset(&slot->cmds);
                {
                    //Create a RenderJob re-using previously cmds-buffer and nullifying its previous GpuJob.
                    ScnRenderJobRef job = ScnRenderJob_alloc(opq->ctx);
                    if(ScnRenderJob_isNull(job)){
                        SCN_PRINTF_ERROR("ScnRender_allocRenderJob::ScnRenderJob_alloc failed.\n");
                    } else if(!ScnRenderJob_swapCmds(job, &slot->cmds, &gpuJob)){
                        SCN_PRINTF_ERROR("ScnRender_allocRenderJob::ScnRenderJob_swapCmds failed.\n");
                    } else {
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
    if(ScnGpuDevice_isNull(opq->gpuDev) || opq->slots.arr == NULL){
        SCN_PRINTF_ERROR("ScnRender_enqueue::ScnGpuDevice or slots are NULL.\n");
    } else {
        //Find an available render slot
        STScnRenderSlot* slot = ScnRender_getAvailRenderSlotLockedOpq_(opq);
        if(slot == NULL){
            SCN_PRINTF_ERROR("ScnRender_enqueue::no available slot.\n");
        } else {
            ScnGpuRenderJobRef gpuJob = ScnGpuDevice_allocRenderJob(opq->gpuDev);
            if(ScnGpuRenderJob_isNull(gpuJob)){
                SCN_PRINTF_ERROR("ScnRender_enqueue::ScnGpuDevice_allocRenderJob failed.\n");
            } else {
                ScnGpuRenderJobRef gpuJobPrev = ScnGpuRenderJobRef_Zero;
                ScnGpuRenderJob_set(&gpuJobPrev, gpuJob);
                if(!ScnRenderJob_swapCmds(job, &slot->cmds, &gpuJobPrev)){
                    SCN_PRINTF_ERROR("ScnRender_enqueue::ScnRenderJob_swapCmds failed.\n");
                } else if(!slot->cmds.isPrepared || ScnMemElastic_isNull(slot->cmds.mPropsScns) || ScnMemElastic_isNull(slot->cmds.mPropsMdls)){
                    SCN_PRINTF_ERROR("ScnRender_enqueue::cmds were not previosuly prepared.\n");
                } else {
                    r = ScnTRUE;
                    SCN_PRINTF_VERB("ScnRenderJob_end::%u objs, %u cmds.\n", opq->cmds.objs.use, opq->cmds.cmds.use);
                    //sync buffers
                    if(r && ScnMemElastic_hasPtrs(slot->cmds.mPropsScns)){
                        if(ScnGpuBuffer_isNull(slot->bPropsScns)){
                            //create new buffer
                            slot->bPropsScns = ScnGpuDevice_allocBuffer(opq->gpuDev, slot->cmds.mPropsScns);
                            if(ScnGpuBuffer_isNull(slot->bPropsScns)){
                                SCN_PRINTF_ERROR("ScnRender_enqueue::ScnGpuBuffer_sync(bPropsScns) failed.\n");
                                r = ScnFALSE;
                            }
                        } else {
                            //sync existing buffer
                            const STScnGpuBufferChanges chngs = STScnGpuBufferChanges_All;
                            if(!ScnGpuBuffer_sync(slot->bPropsScns, slot->cmds.mPropsScns, &chngs)){
                                SCN_PRINTF_ERROR("ScnRender_enqueue::ScnGpuBuffer_sync(bPropsScns) failed.\n");
                            }
                        }
                    }
                    if(r && ScnMemElastic_hasPtrs(slot->cmds.mPropsMdls)){
                        if(ScnGpuBuffer_isNull(slot->bPropsMdls)){
                            //create new buffer
                            slot->bPropsMdls = ScnGpuDevice_allocBuffer(opq->gpuDev, slot->cmds.mPropsMdls);
                            if(ScnGpuBuffer_isNull(slot->bPropsMdls)){
                                SCN_PRINTF_ERROR("ScnRender_enqueue::ScnGpuBuffer_sync(bPropsMdls) failed.\n");
                                r = ScnFALSE;
                            }
                        } else {
                            //sync existing buffer
                            const STScnGpuBufferChanges chngs = STScnGpuBufferChanges_All;
                            if(!ScnGpuBuffer_sync(slot->bPropsMdls, slot->cmds.mPropsMdls, &chngs)){
                                SCN_PRINTF_ERROR("ScnRender_enqueue::ScnGpuBuffer_sync(bPropsMdls) failed.\n");
                            }
                        }
                    }
                    //sync used objects' gpu-data
                    if(r){
                        ScnArraySorted_foreach(&slot->cmds.objs, STScnRenderJobObj, o,
                            switch(o->type){
                                case ENScnRenderJobObjType_Unknown: break;
                                case ENScnRenderJobObjType_Buff:
                                    if(!ScnBuffer_prepareCurrentRenderSlot(o->buff, NULL)){
                                        SCN_PRINTF_ERROR("ScnRenderJob_end::ScnBuffer_prepareCurrentRenderSlot failed.\n");
                                        r = ScnFALSE;
                                        break;
                                    }
                                    break;
                                case ENScnRenderJobObjType_Framebuff:
                                    if(!ScnFramebuff_prepareCurrentRenderSlot(o->framebuff)){
                                        SCN_PRINTF_ERROR("ScnRenderJob_end::ScnFramebuff_prepareCurrentRenderSlot failed.\n");
                                        r = ScnFALSE;
                                        break;
                                    }
                                    break;
                                case ENScnRenderJobObjType_Vertexbuff:
                                    if(!ScnVertexbuff_prepareCurrentRenderSlot(o->vertexbuff)){
                                        SCN_PRINTF_ERROR("ScnRenderJob_end::ScnVertexbuff_prepareCurrentRenderSlot failed.\n");
                                        r = ScnFALSE;
                                        break;
                                    }
                                    break;
                                case ENScnRenderJobObjType_Texture:
                                    if(!ScnTexture_prepareCurrentRenderSlot(o->texture)){
                                        SCN_PRINTF_ERROR("ScnRenderJob_end::ScnTexture_prepareCurrentRenderSlot failed.\n");
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
                    if(r && slot->cmds.cmds.use > 0){
                        if(!ScnGpuRenderJob_buildBegin(gpuJob, slot->bPropsScns, slot->bPropsMdls)){
                            SCN_PRINTF_ERROR("ScnRender_enqueue::ScnGpuRenderJob_buildBegin failed.\n");
                            r = ScnFALSE;
                        } else if(!ScnGpuRenderJob_buildAddCmds(gpuJob, slot->cmds.cmds.arr, slot->cmds.cmds.use)){
                            SCN_PRINTF_ERROR("ScnRender_enqueue::ScnGpuRenderJob_buildAddCmds failed.\n");
                            r = ScnFALSE;
                        } else if(!ScnGpuRenderJob_buildEndAndEnqueue(gpuJob)){
                            SCN_PRINTF_ERROR("ScnRender_enqueue::ScnGpuRenderJob_buildEndAndEnqueue failed.\n");
                            r = ScnFALSE;
                        }
                    }
                    //move used-objects to their next render slot
                    if(r){
                        ScnArraySorted_foreach(&slot->cmds.objs, STScnRenderJobObj, o,
                            switch(o->type){
                                case ENScnRenderJobObjType_Unknown: break;
                                case ENScnRenderJobObjType_Buff:
                                    if(!ScnBuffer_moveToNextRenderSlot(o->buff)){
                                        SCN_PRINTF_ERROR("ScnRenderJob_end::ScnBuffer_moveToNextRenderSlot failed.\n");
                                        r = ScnFALSE;
                                        break;
                                    }
                                    break;
                                case ENScnRenderJobObjType_Framebuff:
                                    if(!ScnFramebuff_moveToNextRenderSlot(o->framebuff)){
                                        SCN_PRINTF_ERROR("ScnRenderJob_end::ScnFramebuff_moveToNextRenderSlot failed.\n");
                                        r = ScnFALSE;
                                        break;
                                    }
                                    break;
                                case ENScnRenderJobObjType_Vertexbuff:
                                    if(!ScnVertexbuff_moveToNextRenderSlot(o->vertexbuff)){
                                        SCN_PRINTF_ERROR("ScnRenderJob_end::ScnVertexbuff_moveToNextRenderSlot failed.\n");
                                        r = ScnFALSE;
                                        break;
                                    }
                                    break;
                                case ENScnRenderJobObjType_Texture:
                                    if(!ScnTexture_moveToNextRenderSlot(o->texture)){
                                        SCN_PRINTF_ERROR("ScnRenderJob_end::ScnTexture_moveToNextRenderSlot failed.\n");
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
                    //set job in slot
                    if(r){
                        ScnGpuRenderJob_set(&slot->gpuJob, gpuJob);
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
        r = ScnRender_allocVertexbuffsLockedOpq_(opq->ctx, opq->gpuDev, opq->gpuDevDesc.memBlockAlign, opq->slots.use, &opq->cfg.mem.verts);
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

ScnVertexbuffsRef ScnRender_allocVertexbuffsLockedOpq_(ScnContextRef ctx, ScnGpuDeviceRef gpuDev, const ScnUI32 memBlockAlign, const ScnUI32 ammRenderSlots, const STScnRenderVertsCfg* cfg){
    ScnVertexbuffsRef rr = ScnObjRef_Zero;
    ScnBOOL r = ScnTRUE;
    if(cfg == NULL){
        return rr;
    }
    ScnBufferRef vBuffs[ENScnVertexType_Count];
    ScnBufferRef iBuffs[ENScnVertexType_Count];
    ScnVertexbuffRef vbs[ENScnVertexType_Count];
    ScnMemory_setZeroSt(vBuffs);
    ScnMemory_setZeroSt(iBuffs);
    ScnMemory_setZeroSt(vbs);
    //initial bufffers
    if(r){
        ScnSI32 i; for(i = 0; i < ENScnVertexType_Count && r; i++){
            ScnUI32 itmSz = 0, ammPerBock = cfg->typesPerBlock[i];
            switch(i){
                case ENScnVertexType_2DColor:   itmSz = sizeof(STScnVertex2D); break;
                case ENScnVertexType_2DTex:     itmSz = sizeof(STScnVertex2DTex); break;
                case ENScnVertexType_2DTex2:    itmSz = sizeof(STScnVertex2DTex2); break;
                case ENScnVertexType_2DTex3:    itmSz = sizeof(STScnVertex2DTex3); break;
                default: SCN_ASSERT(ScnFALSE); r = ScnFALSE; break; //missing implementation
            }
            if(ammPerBock <= 0 || itmSz <= 0 || cfg->idxsPerBlock <= 0){
                r = ScnFALSE;
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
                ammPerBock = cfg->idxsPerBlock;
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
    ScnRenderCmds_reset(&obj->cmds); //for cleaner finalization (tracking user-space's memory leaks)
    ScnRenderCmds_destroy(&obj->cmds);
    ScnGpuRenderJob_releaseAndNull(&obj->gpuJob);
    ScnGpuBuffer_releaseAndNull(&obj->bPropsScns);
    ScnGpuBuffer_releaseAndNull(&obj->bPropsMdls);
}

