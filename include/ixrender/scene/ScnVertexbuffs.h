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

typedef struct STScnVertexIdxPtr_ {
    STScnVertexIdx*     ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    ScnUI32             idx;    //abstract address
} STScnVertexIdxPtr;

//STScnVertexPtr, abstract pointer

#define STScnVertexPtr_Zero { NULL, 0 }

typedef struct STScnVertexPtr_ {
    STScnVertex*        ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    ScnUI32             idx;    //abstract address
} STScnVertexPtr;

//STScnVertexTexPtr, abstract pointer

#define STScnVertexTexPtr_Zero { NULL, 0 }

typedef struct STScnVertexTexPtr_ {
    STScnVertexTex*     ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    ScnUI32             idx;    //abstract address
} STScnVertexTexPtr;

//STScnVertexTex2Ptr, abstract pointer

#define STScnVertexTex2Ptr_Zero { NULL, 0 }

typedef struct STScnVertexTex2Ptr_ {
    STScnVertexTex2*    ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    ScnUI32             idx;    //abstract address
} STScnVertexTex2Ptr;

//STScnVertexTex3Ptr, abstract pointer

#define STScnVertexTex3Ptr_Zero { NULL, 0 }

typedef struct STScnVertexTex3Ptr_ {
    STScnVertexTex3*    ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    ScnUI32             idx;    //abstract address
} STScnVertexTex3Ptr;

//

//ScnVertexbuffsRef

SCN_REF_STRUCT_METHODS_DEC(ScnVertexbuffs)

//Prepare

ScnBOOL         ScnVertexbuffs_prepare(ScnVertexbuffsRef ref, const ScnVertexbuffRef* vBuffs, const ScnUI32 vBuffsSz);

//ENScnVertexType_Color //no texture

STScnVertexPtr  ScnVertexbuffs_v0Alloc(ScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v0Invalidate(ScnVertexbuffsRef ref, const STScnVertexPtr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v0Free(ScnVertexbuffsRef ref, const STScnVertexPtr ptr);
//
STScnVertexIdxPtr ScnVertexbuffs_v0IdxsAlloc(ScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v0IdxsInvalidate(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v0IdxsFree(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr);

//ENScnVertexType_Tex  //one texture

STScnVertexTexPtr ScnVertexbuffs_v1Alloc(ScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v1Invalidate(ScnVertexbuffsRef ref, const STScnVertexTexPtr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v1Free(ScnVertexbuffsRef ref, const STScnVertexTexPtr ptr);
//
STScnVertexIdxPtr ScnVertexbuffs_v1IdxsAlloc(ScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v1IdxsInvalidate(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v1IdxsFree(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr);

//ENScnVertexType_Tex2 //two textures

STScnVertexTex2Ptr ScnVertexbuffs_v2Alloc(ScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v2Invalidate(ScnVertexbuffsRef ref, const STScnVertexTex2Ptr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v2Free(ScnVertexbuffsRef ref, const STScnVertexTex2Ptr ptr);
//
STScnVertexIdxPtr ScnVertexbuffs_v2IdxsAlloc(ScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v2IdxsInvalidate(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v2IdxsFree(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr);

//ENScnVertexType_Tex3 //three textures

STScnVertexTex3Ptr ScnVertexbuffs_v3Alloc(ScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v3Invalidate(ScnVertexbuffsRef ref, const STScnVertexTex3Ptr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v3Free(ScnVertexbuffsRef ref, const STScnVertexTex3Ptr ptr);
//
STScnVertexIdxPtr ScnVertexbuffs_v3IdxsAlloc(ScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v3IdxsInvalidate(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v3IdxsFree(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr);

//gpu-vertexbuffs

ScnBOOL         ScnVertexbuffs_prepareNextRenderSlot(ScnVertexbuffsRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnVertexbuff_h */
