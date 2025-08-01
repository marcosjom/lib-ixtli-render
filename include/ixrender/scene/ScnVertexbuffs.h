//
//  ScnVertexbuff.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnVertexbuffs_h
#define ScnVertexbuffs_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/scene/ScnVertices.h"
#include "ixrender/scene/ScnVertexbuff.h"


#ifdef __cplusplus
extern "C" {
#endif

//STScnVertexIdxPtr, abstract pointer

#define STScnVertexIdxPtr_Zero { NULL, 0 }

typedef struct STScnVertexIdxPtr {
    STScnVertexIdx*     ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    ScnUI32             idx;    //abstract address
} STScnVertexIdxPtr;

//STScnVertex2DPtr, abstract pointer

#define STScnVertex2DPtr_Zero { NULL, 0 }

typedef struct STScnVertex2DPtr {
    STScnVertex2D*      ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    ScnUI32             idx;    //abstract address
} STScnVertex2DPtr;

//STScnVertex2DTexPtr, abstract pointer

#define STScnVertex2DTexPtr_Zero { NULL, 0 }

typedef struct STScnVertex2DTexPtr {
    STScnVertex2DTex*   ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    ScnUI32             idx;    //abstract address
} STScnVertex2DTexPtr;

//STScnVertex2DTex2Ptr, abstract pointer

#define STScnVertex2DTex2Ptr_Zero { NULL, 0 }

typedef struct STScnVertex2DTex2Ptr {
    STScnVertex2DTex2*  ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    ScnUI32             idx;    //abstract address
} STScnVertex2DTex2Ptr;

//STScnVertex2DTex3Ptr, abstract pointer

#define STScnVertex2DTex3Ptr_Zero { NULL, 0 }

typedef struct STScnVertex2DTex3Ptr {
    STScnVertex2DTex3*  ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    ScnUI32             idx;    //abstract address
} STScnVertex2DTex3Ptr;

//

//ScnVertexbuffsRef

SCN_REF_STRUCT_METHODS_DEC(ScnVertexbuffs)

//Prepare

ScnBOOL         ScnVertexbuffs_prepare(ScnVertexbuffsRef ref, const ScnVertexbuffRef* vBuffs, const ScnUI32 vBuffsSz);

ScnVertexbuffRef ScnVertexbuffs_getVertexBuff(ScnVertexbuffsRef ref, const ENScnVertexType type);

//ENScnVertexType_2DColor //no texture

STScnVertex2DPtr ScnVertexbuffs_v0Alloc(ScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v0Invalidate(ScnVertexbuffsRef ref, const STScnVertex2DPtr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v0Free(ScnVertexbuffsRef ref, const STScnVertex2DPtr ptr);
//
STScnVertexIdxPtr ScnVertexbuffs_v0IdxsAlloc(ScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v0IdxsInvalidate(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v0IdxsFree(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr);

//ENScnVertexType_2DTex  //one texture

STScnVertex2DTexPtr ScnVertexbuffs_v1Alloc(ScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v1Invalidate(ScnVertexbuffsRef ref, const STScnVertex2DTexPtr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v1Free(ScnVertexbuffsRef ref, const STScnVertex2DTexPtr ptr);
//
STScnVertexIdxPtr ScnVertexbuffs_v1IdxsAlloc(ScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v1IdxsInvalidate(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v1IdxsFree(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr);

//ENScnVertexType_2DTex2 //two textures

STScnVertex2DTex2Ptr ScnVertexbuffs_v2Alloc(ScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v2Invalidate(ScnVertexbuffsRef ref, const STScnVertex2DTex2Ptr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v2Free(ScnVertexbuffsRef ref, const STScnVertex2DTex2Ptr ptr);
//
STScnVertexIdxPtr ScnVertexbuffs_v2IdxsAlloc(ScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v2IdxsInvalidate(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v2IdxsFree(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr);

//ENScnVertexType_2DTex3 //three textures

STScnVertex2DTex3Ptr ScnVertexbuffs_v3Alloc(ScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v3Invalidate(ScnVertexbuffsRef ref, const STScnVertex2DTex3Ptr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v3Free(ScnVertexbuffsRef ref, const STScnVertex2DTex3Ptr ptr);
//
STScnVertexIdxPtr ScnVertexbuffs_v3IdxsAlloc(ScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v3IdxsInvalidate(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v3IdxsFree(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr);

//gpu-vertexbuffs

ScnBOOL         ScnVertexbuffs_prepareCurrentRenderSlot(ScnVertexbuffsRef ref);
ScnBOOL         ScnVertexbuffs_moveToNextRenderSlot(ScnVertexbuffsRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnVertexbuff_h */
