//
//  ScnVertexbuff.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/scene/ScnVertexbuffs.h"

//STScnVertexbuffsOpq

typedef struct STScnVertexbuffsOpq_ {
    STScnContextRef     ctx;
    STScnMutexRef       mutex;
    STScnVertexbuffRef  vBuffs[ENScnVertexType_Count];
} STScnVertexbuffsOpq;

ScnSI32 ScnVertexbuffs_getOpqSz(void){
    return (ScnSI32)sizeof(STScnVertexbuffsOpq);
}

void ScnVertexbuffs_initZeroedOpq(STScnContextRef ctx, void* obj) {
    STScnVertexbuffsOpq* opq = (STScnVertexbuffsOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex = ScnContext_allocMutex(opq->ctx);
}

void ScnVertexbuffs_destroyOpq(void* obj){
    STScnVertexbuffsOpq* opq = (STScnVertexbuffsOpq*)obj;
    //vBuffs
    {
        STScnVertexbuffRef* b = opq->vBuffs;
        const STScnVertexbuffRef* bAfterEnd = b + ENScnVertexType_Count;
        while(b < bAfterEnd){
            ScnVertexbuff_releaseAndNullify(&*b);
            ++b;
        }
    }
    //
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNullify(&opq->ctx);
}

//prepare

ScnBOOL ScnVertexbuffs_prepare(STScnVertexbuffsRef ref, const STScnVertexbuffRef* vBuffs, const ScnUI32 vBuffsSz){
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
                        case ENScnVertexType_Color:   szPerRecordReq = sizeof(STScnVertex); break;
                        case ENScnVertexType_Tex:     szPerRecordReq = sizeof(STScnVertexTex); break;
                        case ENScnVertexType_Tex2:    szPerRecordReq = sizeof(STScnVertexTex2); break;
                        case ENScnVertexType_Tex3:    szPerRecordReq = sizeof(STScnVertexTex3); break;
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
                STScnVertexbuffRef* b = opq->vBuffs;
                const STScnVertexbuffRef* bAfterEnd = b + ENScnVertexType_Count;
                while(b < bAfterEnd){
                    ScnVertexbuff_releaseAndNullify(&*b);
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

#define ScnVertexbuffs_vNAlloc(V_IDX, PT_CAST_TYPE, GET_BUFF_METHOD)  \
    STScnVertexbuffsOpq* opq = (STScnVertexbuffsOpq*)ScnSharedPtr_getOpq(ref.ptr); \
    ScnMutex_lock(opq->mutex); \
    { \
        STScnVertexbuffRef vb = opq->vBuffs[V_IDX]; \
        if(!ScnVertexbuff_isNull(vb)){ \
            const ScnUI32 szPerRecord = ScnVertexbuff_getSzPerRecord(vb); \
            STScnBufferRef b = GET_BUFF_METHOD(vb); \
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
        STScnVertexbuffRef vb = opq->vBuffs[V_IDX]; \
        if(!ScnVertexbuff_isNull(vb)){ \
            const ScnUI32 szPerRecord = ScnVertexbuff_getSzPerRecord(vb); \
            STScnBufferRef b = GET_BUFF_METHOD(vb); \
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
        STScnVertexbuffRef vb = opq->vBuffs[V_IDX]; \
        if(!ScnVertexbuff_isNull(vb)){ \
            const ScnUI32 szPerRecord = ScnVertexbuff_getSzPerRecord(vb); \
            STScnBufferRef b = GET_BUFF_METHOD(vb); \
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

//ENScnVertexType_Color //no texture

STScnVertexPtr ScnVertexbuffs_v0Alloc(STScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertexPtr r = STScnVertexPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_Color, STScnVertex, ScnVertexbuff_getVertexBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v0Invalidate(STScnVertexbuffsRef ref, const STScnVertexPtr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_Color, ScnVertexbuff_getVertexBuff)
}

ScnBOOL ScnVertexbuffs_v0Free(STScnVertexbuffsRef ref, const STScnVertexPtr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_Color, ScnVertexbuff_getVertexBuff);
}

//

STScnVertexIdxPtr ScnVertexbuffs_v0IdxsAlloc(STScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertexIdxPtr r = STScnVertexPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_Color, STScnVertexIdx, ScnVertexbuff_getIdxsBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v0IdxsInvalidate(STScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_Color, ScnVertexbuff_getIdxsBuff)
}

ScnBOOL ScnVertexbuffs_v0IdxsFree(STScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_Color, ScnVertexbuff_getIdxsBuff);
}

//ENScnVertexType_Tex  //one texture

STScnVertexTexPtr ScnVertexbuffs_v1Alloc(STScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertexTexPtr r = STScnVertexPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_Tex, STScnVertexTex, ScnVertexbuff_getVertexBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v1Invalidate(STScnVertexbuffsRef ref, const STScnVertexTexPtr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_Tex, ScnVertexbuff_getVertexBuff)
}

ScnBOOL ScnVertexbuffs_v1Free(STScnVertexbuffsRef ref, const STScnVertexTexPtr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_Tex, ScnVertexbuff_getVertexBuff);
}

//

STScnVertexIdxPtr ScnVertexbuffs_v1IdxsAlloc(STScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertexIdxPtr r = STScnVertexPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_Tex, STScnVertexIdx, ScnVertexbuff_getIdxsBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v1IdxsInvalidate(STScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_Tex, ScnVertexbuff_getIdxsBuff)
}

ScnBOOL ScnVertexbuffs_v1IdxsFree(STScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_Tex, ScnVertexbuff_getIdxsBuff);
}

//ENScnVertexType_Tex2 //two textures

STScnVertexTex2Ptr ScnVertexbuffs_v2Alloc(STScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertexTex2Ptr r = STScnVertexPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_Tex2, STScnVertexTex2, ScnVertexbuff_getVertexBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v2Invalidate(STScnVertexbuffsRef ref, const STScnVertexTex2Ptr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_Tex2, ScnVertexbuff_getVertexBuff)
}

ScnBOOL ScnVertexbuffs_v2Free(STScnVertexbuffsRef ref, const STScnVertexTex2Ptr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_Tex2, ScnVertexbuff_getVertexBuff);
}

//

STScnVertexIdxPtr ScnVertexbuffs_v2IdxsAlloc(STScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertexIdxPtr r = STScnVertexPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_Tex2, STScnVertexIdx, ScnVertexbuff_getIdxsBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v2IdxsInvalidate(STScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_Tex2, ScnVertexbuff_getIdxsBuff)
}

ScnBOOL ScnVertexbuffs_v2IdxsFree(STScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_Tex2, ScnVertexbuff_getIdxsBuff);
}

//ENScnVertexType_Tex3 //three textures

STScnVertexTex3Ptr ScnVertexbuffs_v3Alloc(STScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertexTex3Ptr r = STScnVertexPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_Tex3, STScnVertexTex3, ScnVertexbuff_getVertexBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v3Invalidate(STScnVertexbuffsRef ref, const STScnVertexTex3Ptr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_Tex3, ScnVertexbuff_getVertexBuff)
}

ScnBOOL ScnVertexbuffs_v3Free(STScnVertexbuffsRef ref, const STScnVertexTex3Ptr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_Tex3, ScnVertexbuff_getVertexBuff);
}

//

STScnVertexIdxPtr ScnVertexbuffs_v3IdxsAlloc(STScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertexIdxPtr r = STScnVertexPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_Tex3, STScnVertexIdx, ScnVertexbuff_getIdxsBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v3IdxsInvalidate(STScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_Tex3, ScnVertexbuff_getIdxsBuff)
}

ScnBOOL ScnVertexbuffs_v3IdxsFree(STScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_Tex3, ScnVertexbuff_getIdxsBuff);
}

//gpu-vertexbuffs

ScnBOOL ScnVertexbuffs_prepareNextRenderSlot(STScnVertexbuffsRef ref){
    ScnBOOL r = ScnFALSE;
    STScnVertexbuffsOpq* opq = (STScnVertexbuffsOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        r = ScnTRUE;
        STScnVertexbuffRef* b = opq->vBuffs;
        const STScnVertexbuffRef* bAfterEnd = b + ENScnVertexType_Count;
        while(b < bAfterEnd){
            if(!ScnVertexbuff_isNull(*b)){
                if(!ScnVertexbuff_prepareNextRenderSlot(*b)){
                    printf("ERROR, ScnVertexbuffs_prepareNextRenderSlot::ScnVertexbuff_prepareNextRenderSlot failed.\n");
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
