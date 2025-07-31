//
//  ScnVertexbuff.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/scene/ScnVertexbuffs.h"

//STScnVertexbuffsOpq

typedef struct STScnVertexbuffsOpq {
    ScnContextRef     ctx;
    ScnMutexRef       mutex;
    ScnVertexbuffRef  vBuffs[ENScnVertexType_Count];
} STScnVertexbuffsOpq;

ScnSI32 ScnVertexbuffs_getOpqSz(void){
    return (ScnSI32)sizeof(STScnVertexbuffsOpq);
}

void ScnVertexbuffs_initZeroedOpq(ScnContextRef ctx, void* obj) {
    STScnVertexbuffsOpq* opq = (STScnVertexbuffsOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex = ScnContext_allocMutex(opq->ctx);
}

void ScnVertexbuffs_destroyOpq(void* obj){
    STScnVertexbuffsOpq* opq = (STScnVertexbuffsOpq*)obj;
    //vBuffs
    {
        ScnVertexbuffRef* b = opq->vBuffs;
        const ScnVertexbuffRef* bAfterEnd = b + ENScnVertexType_Count;
        while(b < bAfterEnd){
            ScnVertexbuff_releaseAndNull(&*b);
            ++b;
        }
    }
    //
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNull(&opq->ctx);
}

//prepare

ScnBOOL ScnVertexbuffs_prepare(ScnVertexbuffsRef ref, const ScnVertexbuffRef* vBuffs, const ScnUI32 vBuffsSz){
    ScnBOOL r = ScnFALSE;
    STScnVertexbuffsOpq* opq = (STScnVertexbuffsOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        r = ScnTRUE;
        //validate
        {
            ScnSI32 i; for(i = 0; r && i < vBuffsSz && i < ENScnVertexType_Count; i++){
                if(!ScnVertexbuff_isNull(vBuffs[i])){
                    const ScnUI32 szPerRecord = ScnVertexbuff_getSzPerRecord(vBuffs[i]);
                    ScnUI32 szPerRecordReq = 0;
                    switch (i) {
                        case ENScnVertexType_2DColor:   szPerRecordReq = sizeof(STScnVertex2D); break;
                        case ENScnVertexType_2DTex:     szPerRecordReq = sizeof(STScnVertex2DTex); break;
                        case ENScnVertexType_2DTex2:    szPerRecordReq = sizeof(STScnVertex2DTex2); break;
                        case ENScnVertexType_2DTex3:    szPerRecordReq = sizeof(STScnVertex2DTex3); break;
                        default: r = ScnFALSE; SCN_ASSERT(ScnFALSE) break;
                    }
                    if(szPerRecord != szPerRecordReq){
                        r = ScnFALSE;
                        break;
                    }
                }
            }
        }
        //apply
        if(r){
            //release
            {
                ScnVertexbuffRef* b = opq->vBuffs;
                const ScnVertexbuffRef* bAfterEnd = b + ENScnVertexType_Count;
                while(b < bAfterEnd){
                    ScnVertexbuff_releaseAndNull(&*b);
                    ++b;
                }
            }
            //set
            {
                ScnSI32 i; for(i = 0; i < vBuffsSz && i < ENScnVertexType_Count; i++){
                    ScnVertexbuff_set(&opq->vBuffs[i], vBuffs[i]);
                }
            }
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnVertexbuffRef ScnVertexbuffs_getVertexBuff(ScnVertexbuffsRef ref, const ENScnVertexType type){
    STScnVertexbuffsOpq* opq = (STScnVertexbuffsOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (type >= 0 && type < ENScnVertexType_Count ? opq->vBuffs[type] : (ScnVertexbuffRef)ScnObjRef_Zero);
}

#define ScnVertexbuffs_vNAlloc(V_IDX, PT_CAST_TYPE, GET_BUFF_METHOD)  \
    STScnVertexbuffsOpq* opq = (STScnVertexbuffsOpq*)ScnSharedPtr_getOpq(ref.ptr); \
    ScnMutex_lock(opq->mutex); \
    { \
        ScnVertexbuffRef vb = opq->vBuffs[V_IDX]; \
        if(!ScnVertexbuff_isNull(vb)){ \
            const ScnUI32 szPerRecord = ScnVertexbuff_getSzPerRecord(vb); \
            ScnBufferRef b = GET_BUFF_METHOD(vb); \
            if(!ScnBuffer_isNull(b)){ \
                STScnAbsPtr r2 = ScnBuffer_malloc(b, amm * szPerRecord); \
                r.idx = r2.idx / szPerRecord; \
                r.ptr = (PT_CAST_TYPE*)r2.ptr; \
            } \
        } \
    } \
    ScnMutex_unlock(opq->mutex);

#define ScnVertexbuffs_vNInvalidate(V_IDX, GET_BUFF_METHOD) \
    ScnBOOL r = ScnFALSE; \
    STScnVertexbuffsOpq* opq = (STScnVertexbuffsOpq*)ScnSharedPtr_getOpq(ref.ptr); \
    ScnMutex_lock(opq->mutex); \
    { \
        ScnVertexbuffRef vb = opq->vBuffs[V_IDX]; \
        if(!ScnVertexbuff_isNull(vb)){ \
            const ScnUI32 szPerRecord = ScnVertexbuff_getSzPerRecord(vb); \
            ScnBufferRef b = GET_BUFF_METHOD(vb); \
            if(!ScnBuffer_isNull(b)){ \
                STScnAbsPtr ptr2 = STScnAbsPtr_Zero; \
                ptr2.idx = ptr.idx * szPerRecord; \
                ptr2.ptr = ptr.ptr; \
                r = ScnBuffer_mInvalidate(b, ptr2, sz * szPerRecord);\
            } \
        } \
    } \
    ScnMutex_unlock(opq->mutex); \
    return r;

#define ScnVertexbuffs_vNFree(V_IDX, GET_BUFF_METHOD) \
    ScnBOOL r = ScnFALSE; \
    STScnVertexbuffsOpq* opq = (STScnVertexbuffsOpq*)ScnSharedPtr_getOpq(ref.ptr); \
    ScnMutex_lock(opq->mutex); \
    { \
        ScnVertexbuffRef vb = opq->vBuffs[V_IDX]; \
        if(!ScnVertexbuff_isNull(vb)){ \
            const ScnUI32 szPerRecord = ScnVertexbuff_getSzPerRecord(vb); \
            ScnBufferRef b = GET_BUFF_METHOD(vb); \
            if(!ScnBuffer_isNull(b)){ \
                STScnAbsPtr ptr2 = STScnAbsPtr_Zero; \
                ptr2.idx = ptr.idx * szPerRecord; \
                ptr2.ptr = ptr.ptr; \
                r = ScnBuffer_mfree(b, ptr2); \
            } \
        } \
    } \
    ScnMutex_unlock(opq->mutex); \
    return r;

//ENScnVertexType_2DColor //no texture

STScnVertex2DPtr ScnVertexbuffs_v0Alloc(ScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertex2DPtr r = STScnVertex2DPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_2DColor, STScnVertex2D, ScnVertexbuff_getVertexBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v0Invalidate(ScnVertexbuffsRef ref, const STScnVertex2DPtr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_2DColor, ScnVertexbuff_getVertexBuff)
}

ScnBOOL ScnVertexbuffs_v0Free(ScnVertexbuffsRef ref, const STScnVertex2DPtr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_2DColor, ScnVertexbuff_getVertexBuff);
}

//

STScnVertexIdxPtr ScnVertexbuffs_v0IdxsAlloc(ScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertexIdxPtr r = STScnVertex2DPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_2DColor, STScnVertexIdx, ScnVertexbuff_getIdxsBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v0IdxsInvalidate(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_2DColor, ScnVertexbuff_getIdxsBuff)
}

ScnBOOL ScnVertexbuffs_v0IdxsFree(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_2DColor, ScnVertexbuff_getIdxsBuff);
}

//ENScnVertexType_2DTex  //one texture

STScnVertex2DTexPtr ScnVertexbuffs_v1Alloc(ScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertex2DTexPtr r = STScnVertex2DPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_2DTex, STScnVertex2DTex, ScnVertexbuff_getVertexBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v1Invalidate(ScnVertexbuffsRef ref, const STScnVertex2DTexPtr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_2DTex, ScnVertexbuff_getVertexBuff)
}

ScnBOOL ScnVertexbuffs_v1Free(ScnVertexbuffsRef ref, const STScnVertex2DTexPtr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_2DTex, ScnVertexbuff_getVertexBuff);
}

//

STScnVertexIdxPtr ScnVertexbuffs_v1IdxsAlloc(ScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertexIdxPtr r = STScnVertex2DPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_2DTex, STScnVertexIdx, ScnVertexbuff_getIdxsBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v1IdxsInvalidate(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_2DTex, ScnVertexbuff_getIdxsBuff)
}

ScnBOOL ScnVertexbuffs_v1IdxsFree(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_2DTex, ScnVertexbuff_getIdxsBuff);
}

//ENScnVertexType_2DTex2 //two textures

STScnVertex2DTex2Ptr ScnVertexbuffs_v2Alloc(ScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertex2DTex2Ptr r = STScnVertex2DPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_2DTex2, STScnVertex2DTex2, ScnVertexbuff_getVertexBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v2Invalidate(ScnVertexbuffsRef ref, const STScnVertex2DTex2Ptr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_2DTex2, ScnVertexbuff_getVertexBuff)
}

ScnBOOL ScnVertexbuffs_v2Free(ScnVertexbuffsRef ref, const STScnVertex2DTex2Ptr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_2DTex2, ScnVertexbuff_getVertexBuff);
}

//

STScnVertexIdxPtr ScnVertexbuffs_v2IdxsAlloc(ScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertexIdxPtr r = STScnVertex2DPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_2DTex2, STScnVertexIdx, ScnVertexbuff_getIdxsBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v2IdxsInvalidate(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_2DTex2, ScnVertexbuff_getIdxsBuff)
}

ScnBOOL ScnVertexbuffs_v2IdxsFree(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_2DTex2, ScnVertexbuff_getIdxsBuff);
}

//ENScnVertexType_2DTex3 //three textures

STScnVertex2DTex3Ptr ScnVertexbuffs_v3Alloc(ScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertex2DTex3Ptr r = STScnVertex2DPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_2DTex3, STScnVertex2DTex3, ScnVertexbuff_getVertexBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v3Invalidate(ScnVertexbuffsRef ref, const STScnVertex2DTex3Ptr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_2DTex3, ScnVertexbuff_getVertexBuff)
}

ScnBOOL ScnVertexbuffs_v3Free(ScnVertexbuffsRef ref, const STScnVertex2DTex3Ptr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_2DTex3, ScnVertexbuff_getVertexBuff);
}

//

STScnVertexIdxPtr ScnVertexbuffs_v3IdxsAlloc(ScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertexIdxPtr r = STScnVertex2DPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_2DTex3, STScnVertexIdx, ScnVertexbuff_getIdxsBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v3IdxsInvalidate(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_2DTex3, ScnVertexbuff_getIdxsBuff)
}

ScnBOOL ScnVertexbuffs_v3IdxsFree(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_2DTex3, ScnVertexbuff_getIdxsBuff);
}

//gpu-vertexbuffs

ScnBOOL ScnVertexbuffs_prepareCurrentRenderSlot(ScnVertexbuffsRef ref){
    ScnBOOL r = ScnFALSE;
    STScnVertexbuffsOpq* opq = (STScnVertexbuffsOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        r = ScnTRUE;
        ScnVertexbuffRef* b = opq->vBuffs;
        const ScnVertexbuffRef* bAfterEnd = b + ENScnVertexType_Count;
        while(b < bAfterEnd){
            if(!ScnVertexbuff_isNull(*b)){
                if(!ScnVertexbuff_prepareCurrentRenderSlot(*b)){
                    printf("ERROR, ScnVertexbuffs_prepareCurrentRenderSlot::ScnVertexbuff_prepareCurrentRenderSlot failed.\n");
                    r = ScnFALSE;
                    break;
                }
            }
            ++b;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

ScnBOOL ScnVertexbuffs_moveToNextRenderSlot(ScnVertexbuffsRef ref){
    ScnBOOL r = ScnFALSE;
    STScnVertexbuffsOpq* opq = (STScnVertexbuffsOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        r = ScnTRUE;
        ScnVertexbuffRef* b = opq->vBuffs;
        const ScnVertexbuffRef* bAfterEnd = b + ENScnVertexType_Count;
        while(b < bAfterEnd){
            if(!ScnVertexbuff_isNull(*b)){
                if(!ScnVertexbuff_moveToNextRenderSlot(*b)){
                    printf("ERROR, ScnVertexbuffs_prepareCurrentRenderSlot::ScnVertexbuff_moveToNextRenderSlot failed.\n");
                    r = ScnFALSE;
                    break;
                }
            }
            ++b;
        }
    }
    ScnMutex_unlock(opq->mutex);
    return r;
}

