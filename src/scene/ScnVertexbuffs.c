//
//  ScnVertexbuff.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/scene/ScnVertexbuffs.h"

//STScnVertexbuffsOpq

typedef struct STScnVertexbuffsOpq_ {
    STScnContextRef         ctx;
    STScnMutexRef           mutex;
    STScnGpuVertexbuffRef   vBuffs[ENScnVertexType_Count];
} STScnVertexbuffsOpq;

ScnSI32 ScnVertexbuffs_getOpqSz(void){
    return (ScnSI32)sizeof(STScnVertexbuffsOpq);
}

void ScnVertexbuffs_initZeroedOpq(STScnContextRef ctx, void* obj) {
    STScnVertexbuffsOpq* opq = (STScnVertexbuffsOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex = ScnContext_mutex_alloc(opq->ctx);
}

void ScnVertexbuffs_destroyOpq(void* obj){
    STScnVertexbuffsOpq* opq = (STScnVertexbuffsOpq*)obj;
    //vBuffs
    {
        STScnGpuVertexbuffRef* b = opq->vBuffs;
        const STScnGpuVertexbuffRef* bAfterEnd = b + ENScnVertexType_Count;
        while(b < bAfterEnd){
            if(!ScnGpuVertexbuff_isNull(*b)){
                ScnGpuVertexbuff_release(b);
                ScnGpuVertexbuff_null(b);
            }
            ++b;
        }
    }
    //
    if(!ScnMutex_isNull(opq->mutex)){
        ScnMutex_free(&opq->mutex);
        ScnMutex_null(&opq->mutex);
    }
    //
    if(!ScnContext_isNull(opq->ctx)){
        ScnContext_release(&opq->ctx);
        ScnContext_null(&opq->ctx);
    }
}

//prepare

ScnBOOL ScnVertexbuffs_prepare(STScnVertexbuffsRef ref, const STScnGpuVertexbuffRef* vBuffs, const ScnUI32 vBuffsSz){
    ScnBOOL r = Scn_FALSE;
    STScnVertexbuffsOpq* opq = (STScnVertexbuffsOpq*)ScnSharedPtr_getOpq(ref.ptr);
    ScnMutex_lock(opq->mutex);
    {
        r = Scn_TRUE;
        //validate
        {
            ScnSI32 i; for(i = 0; r && i < vBuffsSz && i < ENScnVertexType_Count; i++){
                if(!ScnGpuVertexbuff_isNull(vBuffs[i])){
                    const ScnUI32 szPerRecord = ScnGpuVertexbuff_getSzPerRecord(vBuffs[i]);
                    ScnUI32 szPerRecordReq = 0;
                    switch (i) {
                        case ENScnVertexType_Color:   szPerRecordReq = sizeof(STScnVertex); break;
                        case ENScnVertexType_Tex:     szPerRecordReq = sizeof(STScnVertexTex); break;
                        case ENScnVertexType_Tex2:    szPerRecordReq = sizeof(STScnVertexTex2); break;
                        case ENScnVertexType_Tex3:    szPerRecordReq = sizeof(STScnVertexTex3); break;
                        default: r = Scn_FALSE; SCN_ASSERT(Scn_FALSE) break;
                    }
                    if(szPerRecord != szPerRecordReq){
                        r = Scn_FALSE;
                        break;
                    }
                }
            }
        }
        //apply
        if(r){
            //release
            {
                STScnGpuVertexbuffRef* b = opq->vBuffs;
                const STScnGpuVertexbuffRef* bAfterEnd = b + ENScnVertexType_Count;
                while(b < bAfterEnd){
                    if(!ScnGpuVertexbuff_isNull(*b)){
                        ScnGpuVertexbuff_release(b);
                        ScnGpuVertexbuff_null(b);
                    }
                    ++b;
                }
            }
            //set
            {
                ScnSI32 i; for(i = 0; i < vBuffsSz && i < ENScnVertexType_Count; i++){
                    ScnGpuVertexbuff_set(&opq->vBuffs[i], vBuffs[i]);
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
        STScnGpuVertexbuffRef vb = opq->vBuffs[V_IDX]; \
        if(!ScnGpuVertexbuff_isNull(vb)){ \
            const ScnUI32 szPerRecord = ScnGpuVertexbuff_getSzPerRecord(vb); \
            STScnGpuBufferRef b = GET_BUFF_METHOD(vb); \
            if(!ScnGpuBuffer_isNull(b)){ \
                STScnAbsPtr r2 = ScnGpuBuffer_malloc(b, amm * szPerRecord); \
                r.idx = r2.idx; \
                r.ptr = (PT_CAST_TYPE*)r2.ptr; \
            } \
        } \
    } \
    ScnMutex_unlock(opq->mutex);

#define ScnVertexbuffs_vNInvalidate(V_IDX, GET_BUFF_METHOD) \
    ScnBOOL r = Scn_FALSE; \
    STScnVertexbuffsOpq* opq = (STScnVertexbuffsOpq*)ScnSharedPtr_getOpq(ref.ptr); \
    ScnMutex_lock(opq->mutex); \
    { \
        STScnGpuVertexbuffRef vb = opq->vBuffs[V_IDX]; \
        if(!ScnGpuVertexbuff_isNull(vb)){ \
            const ScnUI32 szPerRecord = ScnGpuVertexbuff_getSzPerRecord(vb); \
            STScnGpuBufferRef b = GET_BUFF_METHOD(vb); \
            if(!ScnGpuBuffer_isNull(b)){ \
                STScnAbsPtr ptr2 = STScnAbsPtr_Zero; \
                ptr2.idx = ptr.idx; \
                ptr2.ptr = ptr.ptr; \
                r = ScnGpuBuffer_mInvalidate(b, ptr2, sz * szPerRecord);\
            } \
        } \
    } \
    ScnMutex_unlock(opq->mutex); \
    return r;

#define ScnVertexbuffs_vNFree(V_IDX, GET_BUFF_METHOD) \
    ScnBOOL r = Scn_FALSE; \
    STScnVertexbuffsOpq* opq = (STScnVertexbuffsOpq*)ScnSharedPtr_getOpq(ref.ptr); \
    ScnMutex_lock(opq->mutex); \
    { \
        STScnGpuVertexbuffRef vb = opq->vBuffs[V_IDX]; \
        if(!ScnGpuVertexbuff_isNull(vb)){ \
            STScnGpuBufferRef b = GET_BUFF_METHOD(vb); \
            if(!ScnGpuBuffer_isNull(b)){ \
                STScnAbsPtr ptr2 = STScnAbsPtr_Zero; \
                ptr2.idx = ptr.idx; \
                ptr2.ptr = ptr.ptr; \
                r = ScnGpuBuffer_mfree(b, ptr2); \
            } \
        } \
    } \
    ScnMutex_unlock(opq->mutex); \
    return r;

//ENScnVertexType_Color //no texture

STScnVertexPtr ScnVertexbuffs_v0Alloc(STScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertexPtr r = STScnVertexPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_Color, STScnVertex, ScnGpuVertexbuff_getVertexBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v0Invalidate(STScnVertexbuffsRef ref, const STScnVertexPtr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_Color, ScnGpuVertexbuff_getVertexBuff)
}

ScnBOOL ScnVertexbuffs_v0Free(STScnVertexbuffsRef ref, const STScnVertexPtr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_Color, ScnGpuVertexbuff_getVertexBuff);
}

//

STScnVertexIdxPtr ScnVertexbuffs_v0IdxsAlloc(STScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertexIdxPtr r = STScnVertexPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_Color, STScnVertexIdx, ScnGpuVertexbuff_getIdxsBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v0IdxsInvalidate(STScnVertexbuffsRef ref, const STScnVertexPtr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_Color, ScnGpuVertexbuff_getIdxsBuff)
}

ScnBOOL ScnVertexbuffs_v0IdxsFree(STScnVertexbuffsRef ref, const STScnVertexPtr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_Color, ScnGpuVertexbuff_getIdxsBuff);
}

//ENScnVertexType_Tex  //one texture

STScnVertexTexPtr ScnVertexbuffs_v1Alloc(STScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertexTexPtr r = STScnVertexPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_Tex, STScnVertexTex, ScnGpuVertexbuff_getVertexBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v1Invalidate(STScnVertexbuffsRef ref, const STScnVertexPtr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_Tex, ScnGpuVertexbuff_getVertexBuff)
}

ScnBOOL ScnVertexbuffs_v1Free(STScnVertexbuffsRef ref, const STScnVertexPtr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_Tex, ScnGpuVertexbuff_getVertexBuff);
}

//

STScnVertexIdxPtr ScnVertexbuffs_v1IdxsAlloc(STScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertexIdxPtr r = STScnVertexPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_Tex, STScnVertexIdx, ScnGpuVertexbuff_getIdxsBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v1IdxsInvalidate(STScnVertexbuffsRef ref, const STScnVertexPtr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_Tex, ScnGpuVertexbuff_getIdxsBuff)
}

ScnBOOL ScnVertexbuffs_v1IdxsFree(STScnVertexbuffsRef ref, const STScnVertexPtr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_Tex, ScnGpuVertexbuff_getIdxsBuff);
}

//ENScnVertexType_Tex2 //two textures

STScnVertexTex2Ptr ScnVertexbuffs_v2Alloc(STScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertexTex2Ptr r = STScnVertexPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_Tex2, STScnVertexTex2, ScnGpuVertexbuff_getVertexBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v2Invalidate(STScnVertexbuffsRef ref, const STScnVertexPtr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_Tex2, ScnGpuVertexbuff_getVertexBuff)
}

ScnBOOL ScnVertexbuffs_v2Free(STScnVertexbuffsRef ref, const STScnVertexPtr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_Tex2, ScnGpuVertexbuff_getVertexBuff);
}

//

STScnVertexIdxPtr ScnVertexbuffs_v2IdxsAlloc(STScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertexIdxPtr r = STScnVertexPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_Tex2, STScnVertexIdx, ScnGpuVertexbuff_getIdxsBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v2IdxsInvalidate(STScnVertexbuffsRef ref, const STScnVertexPtr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_Tex2, ScnGpuVertexbuff_getIdxsBuff)
}

ScnBOOL ScnVertexbuffs_v2IdxsFree(STScnVertexbuffsRef ref, const STScnVertexPtr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_Tex2, ScnGpuVertexbuff_getIdxsBuff);
}

//ENScnVertexType_Tex3 //three textures

STScnVertexTex3Ptr ScnVertexbuffs_v3Alloc(STScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertexTex3Ptr r = STScnVertexPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_Tex3, STScnVertexTex3, ScnGpuVertexbuff_getVertexBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v3Invalidate(STScnVertexbuffsRef ref, const STScnVertexPtr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_Tex3, ScnGpuVertexbuff_getVertexBuff)
}

ScnBOOL ScnVertexbuffs_v3Free(STScnVertexbuffsRef ref, const STScnVertexPtr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_Tex3, ScnGpuVertexbuff_getVertexBuff);
}

//

STScnVertexIdxPtr ScnVertexbuffs_v3IdxsAlloc(STScnVertexbuffsRef ref, const ScnUI32 amm){
    STScnVertexIdxPtr r = STScnVertexPtr_Zero;
    ScnVertexbuffs_vNAlloc(ENScnVertexType_Tex3, STScnVertexIdx, ScnGpuVertexbuff_getIdxsBuff)
    return r;
}

ScnBOOL ScnVertexbuffs_v3IdxsInvalidate(STScnVertexbuffsRef ref, const STScnVertexPtr ptr, const ScnUI32 sz){
    ScnVertexbuffs_vNInvalidate(ENScnVertexType_Tex3, ScnGpuVertexbuff_getIdxsBuff)
}

ScnBOOL ScnVertexbuffs_v3IdxsFree(STScnVertexbuffsRef ref, const STScnVertexPtr ptr){
    ScnVertexbuffs_vNFree(ENScnVertexType_Tex3, ScnGpuVertexbuff_getIdxsBuff);
}
