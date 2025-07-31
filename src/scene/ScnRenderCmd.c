//
//  ScnRenderCmd.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/scene/ScnRenderCmd.h"

//STScnModelDrawCmd

void ScnModelDrawCmd_init(STScnModelDrawCmd* obj){
    memset(obj, 0, sizeof(*obj));
}

void ScnModelDrawCmd_destroy(STScnModelDrawCmd* obj){
    //idxs
    //verts
    SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs) || (obj->verts.count == 0 && obj->verts.v0.ptr == NULL && obj->verts.v0.idx == 0 && obj->idxs.count == 0 && obj->idxs.i0.ptr == NULL && obj->idxs.i0.idx == 0))
    switch(obj->type){
        case ENScnModelDrawCmdType_v0:
            SCN_ASSERT(obj->idxs.count == 0 && obj->idxs.i0.ptr == NULL && obj->idxs.i0.idx == 0)
            if(obj->verts.v0.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v0Free(obj->vbuffs, obj->verts.v0);
                }
                obj->verts.v0 = (STScnVertexPtr)STScnVertexPtr_Zero;
            }
            break;
        case ENScnModelDrawCmdType_v1:
            SCN_ASSERT(obj->idxs.count == 0 && obj->idxs.i1.ptr == NULL && obj->idxs.i1.idx == 0)
            if(obj->verts.v1.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v1Free(obj->vbuffs, obj->verts.v1);
                }
                obj->verts.v1 = (STScnVertexTexPtr)STScnVertexTexPtr_Zero;
            }
            break;
        case ENScnModelDrawCmdType_v2:
            SCN_ASSERT(obj->idxs.count == 0 && obj->idxs.i2.ptr == NULL && obj->idxs.i2.idx == 0)
            if(obj->verts.v2.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v2Free(obj->vbuffs, obj->verts.v2);
                }
                obj->verts.v2 = (STScnVertexTex2Ptr)STScnVertexTex2Ptr_Zero;
            }
            break;
        case ENScnModelDrawCmdType_v3:
            SCN_ASSERT(obj->idxs.count == 0 && obj->idxs.i3.ptr == NULL && obj->idxs.i3.idx == 0)
            if(obj->verts.v3.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v3Free(obj->vbuffs, obj->verts.v3);
                }
                obj->verts.v3 = (STScnVertexTex3Ptr)STScnVertexTex3Ptr_Zero;
            }
            break;
            //
        case ENScnModelDrawCmdType_i0:
            if(obj->verts.v0.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v0Free(obj->vbuffs, obj->verts.v0);
                }
                obj->verts.v0 = (STScnVertexPtr)STScnVertexPtr_Zero;
            }
            if(obj->idxs.i0.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v0IdxsFree(obj->vbuffs, obj->idxs.i0);
                }
                obj->idxs.i0 = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
            }
            break;
        case ENScnModelDrawCmdType_i1:
            if(obj->verts.v1.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v1Free(obj->vbuffs, obj->verts.v1);
                }
                obj->verts.v1 = (STScnVertexTexPtr)STScnVertexTexPtr_Zero;
            }
            if(obj->idxs.i1.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v1IdxsFree(obj->vbuffs, obj->idxs.i1);
                }
                obj->idxs.i1 = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
            }
            break;
        case ENScnModelDrawCmdType_i2:
            if(obj->verts.v2.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v2Free(obj->vbuffs, obj->verts.v2);
                }
                obj->verts.v2 = (STScnVertexTex2Ptr)STScnVertexTex2Ptr_Zero;
            }
            if(obj->idxs.i2.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v2IdxsFree(obj->vbuffs, obj->idxs.i2);
                }
                obj->idxs.i2 = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
            }
            break;
        case ENScnModelDrawCmdType_i3:
            if(obj->verts.v3.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v3Free(obj->vbuffs, obj->verts.v3);
                }
                obj->verts.v3 = (STScnVertexTex3Ptr)STScnVertexTex3Ptr_Zero;
            }
            if(obj->idxs.i3.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v3IdxsFree(obj->vbuffs, obj->idxs.i3);
                }
                obj->idxs.i3 = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
            }
            break;
        default:
            SCN_ASSERT(obj->verts.count == 0 && obj->verts.v0.ptr == NULL && obj->verts.v0.idx == 0 && obj->idxs.count == 0 && obj->idxs.i0.ptr == NULL && obj->idxs.i0.idx == 0)
            break;
    }
    //texs
    {
        ScnGpuTexture_releaseAndNullify(&obj->texs.t0);
        ScnGpuTexture_releaseAndNullify(&obj->texs.t1);
        ScnGpuTexture_releaseAndNullify(&obj->texs.t2);
    }
    //vbuffs
    ScnVertexbuffs_releaseAndNullify(&obj->vbuffs);
}
