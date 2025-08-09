//
//  ScnRenderCmd.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/scene/ScnRenderCmd.h"

//STScnModel2dCmd

void ScnModel2dCmd_init(STScnModel2dCmd* obj){
    ScnMemory_setZeroSt(*obj);
}

void ScnModel2dCmd_destroy(STScnModel2dCmd* obj){
    //idxs
    //verts
    SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs) || (obj->verts.count == 0 && obj->verts.v0.ptr == NULL && obj->verts.v0.idx == 0 && obj->idxs.count == 0 && obj->idxs.i0.ptr == NULL && obj->idxs.i0.idx == 0))
    switch(obj->type){
        case ENScnModelDrawCmdType_2Dv0:
            SCN_ASSERT(obj->idxs.count == 0 && obj->idxs.i0.ptr == NULL && obj->idxs.i0.idx == 0)
            if(obj->verts.v0.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v0Free(obj->vbuffs, obj->verts.v0);
                }
                obj->verts.v0 = (STScnVertex2DPtr)STScnVertex2DPtr_Zero;
            }
            break;
        case ENScnModelDrawCmdType_2Dv1:
            SCN_ASSERT(obj->idxs.count == 0 && obj->idxs.i1.ptr == NULL && obj->idxs.i1.idx == 0)
            if(obj->verts.v1.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v1Free(obj->vbuffs, obj->verts.v1);
                }
                obj->verts.v1 = (STScnVertex2DTexPtr)STScnVertex2DTexPtr_Zero;
            }
            break;
        case ENScnModelDrawCmdType_2Dv2:
            SCN_ASSERT(obj->idxs.count == 0 && obj->idxs.i2.ptr == NULL && obj->idxs.i2.idx == 0)
            if(obj->verts.v2.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v2Free(obj->vbuffs, obj->verts.v2);
                }
                obj->verts.v2 = (STScnVertex2DTex2Ptr)STScnVertex2DTex2Ptr_Zero;
            }
            break;
        case ENScnModelDrawCmdType_2Dv3:
            SCN_ASSERT(obj->idxs.count == 0 && obj->idxs.i3.ptr == NULL && obj->idxs.i3.idx == 0)
            if(obj->verts.v3.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v3Free(obj->vbuffs, obj->verts.v3);
                }
                obj->verts.v3 = (STScnVertex2DTex3Ptr)STScnVertex2DTex3Ptr_Zero;
            }
            break;
            //
        case ENScnModelDrawCmdType_2Di0:
            if(obj->verts.v0.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v0Free(obj->vbuffs, obj->verts.v0);
                }
                obj->verts.v0 = (STScnVertex2DPtr)STScnVertex2DPtr_Zero;
            }
            if(obj->idxs.i0.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v0IdxsFree(obj->vbuffs, obj->idxs.i0);
                }
                obj->idxs.i0 = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
            }
            break;
        case ENScnModelDrawCmdType_2Di1:
            if(obj->verts.v1.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v1Free(obj->vbuffs, obj->verts.v1);
                }
                obj->verts.v1 = (STScnVertex2DTexPtr)STScnVertex2DTexPtr_Zero;
            }
            if(obj->idxs.i1.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v1IdxsFree(obj->vbuffs, obj->idxs.i1);
                }
                obj->idxs.i1 = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
            }
            break;
        case ENScnModelDrawCmdType_2Di2:
            if(obj->verts.v2.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v2Free(obj->vbuffs, obj->verts.v2);
                }
                obj->verts.v2 = (STScnVertex2DTex2Ptr)STScnVertex2DTex2Ptr_Zero;
            }
            if(obj->idxs.i2.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v2IdxsFree(obj->vbuffs, obj->idxs.i2);
                }
                obj->idxs.i2 = (STScnVertexIdxPtr)STScnVertexIdxPtr_Zero;
            }
            break;
        case ENScnModelDrawCmdType_2Di3:
            if(obj->verts.v3.ptr != NULL){
                SCN_ASSERT(!ScnVertexbuffs_isNull(obj->vbuffs))
                if(!ScnVertexbuffs_isNull(obj->vbuffs)){
                    ScnVertexbuffs_v3Free(obj->vbuffs, obj->verts.v3);
                }
                obj->verts.v3 = (STScnVertex2DTex3Ptr)STScnVertex2DTex3Ptr_Zero;
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
        ScnTexture_releaseAndNull(&obj->texs[0]);
        ScnTexture_releaseAndNull(&obj->texs[1]);
        ScnTexture_releaseAndNull(&obj->texs[2]);
        SCN_ASSERT(ENScnGpuTextureIdx_Count == (sizeof(obj->texs) / sizeof(obj->texs[0])))
    }
    //vbuffs
    ScnVertexbuffs_releaseAndNull(&obj->vbuffs);
}
