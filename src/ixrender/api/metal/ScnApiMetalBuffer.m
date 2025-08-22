//
//  ScnApiMetalBuffer.m
//  ixtli-render
//
//  Created by Marcos Ortega on 18/8/25.
//

#include "ScnApiMetalBuffer.h"

#ifdef SCN_ASSERTS_ACTIVATED
#   include <string.h> //strncmp()
#endif

void ScnApiMetalBuffer_free(void* pObj){
    STScnApiMetalBuffer* obj = (STScnApiMetalBuffer*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        if(obj->buff != nil){
            [obj->buff release];
            obj->buff = nil;
        }
        if(obj->dev != nil){
            //release?
            obj->dev = nil;
        }
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

#ifdef SCN_ASSERTS_ACTIVATED
ScnBOOL ScnApiMetalBuffer_syncValidate_(STScnApiMetalBuffer* obj, ScnMemElasticRef mem);
#endif

ScnBOOL ScnApiMetalBuffer_sync(void* pObj, ScnMemElasticRef mem, const STScnGpuBufferChanges* changes){
    ScnBOOL r = ScnFALSE;
    STScnApiMetalBuffer* obj = (STScnApiMetalBuffer*)pObj;
    if(obj->buff == nil || ScnMemElastic_isNull(mem)){
        //missing params
        return ScnFALSE;
    }
    //sync
    {
        ScnBOOL buffIsNew = ScnFALSE;
        ScnUI32 buffLen = (ScnUI32)obj->buff.length;
        const ScnUI32 cpuBuffSz = ScnMemElastic_getAddressableSize(mem);
        //resize (if necesary)
        if(cpuBuffSz != buffLen){
            //recreate buffer
            id<MTLBuffer> buff = [obj->dev newBufferWithLength:cpuBuffSz options:MTLResourceStorageModeShared | MTLResourceCPUCacheModeWriteCombined];
            if(buff != nil){
                SCN_PRINTF_VERB("ScnApiMetalBuffer_sync::gpu-buff resized from %u to %u bytes.\n", buffLen, cpuBuffSz);
                buffLen = cpuBuffSz;
                [obj->buff release];
                obj->buff = buff;
                buffIsNew = ScnTRUE;
            }
        }
        //sync
        if(buffLen < cpuBuffSz){
            SCN_PRINTF_ERROR("ScnApiMetalBuffer_sync::gpuBuff is smaller than cpu-buff.\n");
        } else if(buffIsNew || changes->all){
            //sync all
            const STScnRangeU rngAll = ScnMemElastic_getUsedAddressesRngAligned(mem);
            if(!ScnApiMetalBuffer_syncRanges_(obj->buff, mem, &rngAll, 1)){
                SCN_PRINTF_ERROR("ScnApiMetalBuffer_sync::ScnApiMetalBuffer_syncRanges_ failed.\n");
            } else {
                r = ScnTRUE;
                //validate
                //SCN_ASSERT(ScnApiMetalBuffer_syncValidate_(obj, mem))
            }
        } else {
            //sync ranges only
            if(!ScnApiMetalBuffer_syncRanges_(obj->buff, mem, changes->rngs, changes->rngsUse)){
                SCN_PRINTF_ERROR("ScnApiMetalBuffer_sync::ScnApiMetalBuffer_syncRanges_ failed.\n");
            } else {
                r = ScnTRUE;
                //validate
                //SCN_ASSERT(ScnApiMetalBuffer_syncValidate_(obj, mem))
            }
        }
    }
    return r;
}

#ifdef SCN_ASSERTS_ACTIVATED
typedef struct ScnApiMetalBuffer_syncValidate_st {
    STScnApiMetalBuffer*    obj;
    STScnRangeU             usedAddressesRng;
    ScnUI32                 leftLimitFnd;
    ScnUI32                 rghtLimitFnd;
} ScnApiMetalBuffer_syncValidate_st;
#endif

#ifdef SCN_ASSERTS_ACTIVATED
ScnBOOL ScnApiMetalBuffer_syncValidate_pushBlockPtrs_(void* data, const ScnUI32 rootIndex, const void* rootAddress, const STScnMemBlockPtr* const ptrs, const ScnUI32 ptrsSz){
    ScnBOOL r = ScnFALSE;
    ScnApiMetalBuffer_syncValidate_st* st = (ScnApiMetalBuffer_syncValidate_st*)data;
    STScnApiMetalBuffer* obj = st->obj;
    const STScnRangeU usedAddressesRng = st->usedAddressesRng;
    if(obj->buff != nil){
        const ScnUI32 buffLen       = (ScnUI32)obj->buff.length;
        ScnBYTE* buffPtr            = (ScnBYTE*)obj->buff.contents;
        const STScnMemBlockPtr* ptr = ptrs;
        const STScnMemBlockPtr* ptrAfterEnd = ptr + ptrsSz;
        const ScnUI32 usedAddressesRngAfterEnd = usedAddressesRng.start + usedAddressesRng.size;
        r = ScnTRUE;
        while(ptr < ptrAfterEnd){
            const ScnUI32 ptrIdx = rootIndex + (ScnUI32)((const ScnBYTE*)ptr->ptr - (const ScnBYTE*)rootAddress);
            const ScnUI32 ptrIdxAfterEnd = ptrIdx + ptr->sz;
            if(ptrIdx < usedAddressesRng.start || ptrIdxAfterEnd > usedAddressesRngAfterEnd){
                SCN_ASSERT(ScnFALSE) //pointer out of ScnMemElastic's declared acitve range (program logic error)
                r = ScnFALSE;
                break;
            } else if(ptrIdxAfterEnd > buffLen){
                SCN_ASSERT(ScnFALSE) //pointer out of buffer's range
                r = ScnFALSE;
                break;
            } else if(ptr->sz > 0 && 0 != strncmp((const char*)&buffPtr[ptrIdx], ptr->ptr, ptr->sz)){
                SCN_ASSERT(ScnFALSE) //data missmatch
                r = ScnFALSE;
                break;
            }
            if(ptrIdx <= st->leftLimitFnd){
                SCN_ASSERT(ptrIdx < st->leftLimitFnd) //ptrs should not repeat themself
                st->leftLimitFnd = ptrIdx;
            }
            if(ptrIdxAfterEnd >= st->rghtLimitFnd){
                SCN_ASSERT(ptrIdxAfterEnd > st->rghtLimitFnd) //ptrs should not repeat themself
                st->rghtLimitFnd = ptrIdxAfterEnd;
            }
            //next
            ++ptr;
        }
    }
    return r;
}
#endif

#ifdef SCN_ASSERTS_ACTIVATED
ScnBOOL ScnApiMetalBuffer_syncValidate_(STScnApiMetalBuffer* obj, ScnMemElasticRef mem){
    ScnBOOL r = ScnTRUE;
    ScnApiMetalBuffer_syncValidate_st st;
    ScnMemory_setZeroSt(st);
    st.obj              = obj;
    st.usedAddressesRng = ScnMemElastic_getUsedAddressesRng(mem);
    st.leftLimitFnd     = 0xFFFFFFFFu;
    st.rghtLimitFnd     = 0;
    {
        r = ScnMemElastic_pushPtrs(mem, ScnApiMetalBuffer_syncValidate_pushBlockPtrs_, &st);
    }
    SCN_ASSERT(st.leftLimitFnd == st.usedAddressesRng.start && st.rghtLimitFnd == (st.usedAddressesRng.start + st.usedAddressesRng.size)) //program logic error, the used-address-rng notified by the elastic-memory was miscalculated
    return r;
}
#endif

ScnBOOL ScnApiMetalBuffer_syncRanges_(id<MTLBuffer> buff, ScnMemElasticRef mem, const STScnRangeU* const rngs, const ScnUI32 rngsUse){
    ScnBOOL r = ScnTRUE;
    if(rngsUse <= 0){
        //nothing to sync
        return r;
    }
    //
    const ScnUI32 buffLen = (ScnUI32)buff.length;
    ScnBYTE* buffPtr = (ScnBYTE*)buff.contents;
    STScnAbsPtr ptr = STScnAbsPtr_Zero;
    ScnUI32 continuousSz = 0, copySz = 0, bytesCopied = 0;
    //
#   ifdef SCN_ASSERTS_ACTIVATED
    ScnUI32 prevRngAfterEnd = 0;
#   endif
    const STScnRangeU* rng = rngs;
    const STScnRangeU* rngAfterEnd = rngs + rngsUse;
    STScnRangeU curRng;
    while(rng < rngAfterEnd && r){
        SCN_ASSERT(prevRngAfterEnd <= rng->start + rng->size) //rngs should be ordered and non-overlapping
        //copy range data
        curRng = *rng;
        while(curRng.size > 0){
            ptr = ScnMemElastic_getNextContinuousAddress(mem, curRng.start, &continuousSz);
            if(ptr.ptr == NULL){
                break;
            }
            SCN_ASSERT(ptr.idx == curRng.start);
            SCN_ASSERT((curRng.start + continuousSz) <= buffLen);
            if((curRng.start + continuousSz) > buffLen){
                SCN_PRINTF_ERROR("gpu-buffer is smaller than cpu-buffer.\n");
                r = ScnFALSE;
                break;
            }
            //copy
            copySz = (curRng.size < continuousSz ? curRng.size : continuousSz);
            ScnMemcpy(&buffPtr[curRng.start], ptr.ptr, copySz);
            bytesCopied += copySz;
            //
            SCN_ASSERT(curRng.size >= copySz)
            curRng.start += copySz;
            curRng.size -= copySz;
        }
#       ifdef SCN_ASSERTS_ACTIVATED
        prevRngAfterEnd = rng->start + rng->size;
#       endif
        //next
        ++rng;
    }
    SCN_PRINTF_VERB("%2.f%% %u of %u bytes synced at buffer.\n", (float)bytesCopied * 100.f / (float)buffLen, bytesCopied, buffLen);
    return r;
}
