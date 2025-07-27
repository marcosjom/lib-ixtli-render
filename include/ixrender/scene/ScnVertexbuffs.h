//
//  ScnVertexbuff.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnVertexbuff_h
#define ScnVertexbuff_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/gpu/ScnGpuVertexbuff.h"
#include "ixrender/scene/ScnVertices.h"


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

//STScnVertexbuffsRef

SCN_REF_STRUCT_METHODS_DEC(ScnVertexbuffs)

//Prepare

ScnBOOL         ScnVertexbuffs_prepare(STScnVertexbuffsRef ref, const STScnGpuVertexbuffRef* vBuffs, const ScnUI32 vBuffsSz);

//ENScnVertexType_Color //no texture

STScnVertexPtr  ScnVertexbuffs_v0Alloc(STScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v0Invalidate(STScnVertexbuffsRef ref, const STScnVertexPtr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v0Free(STScnVertexbuffsRef ref, const STScnVertexPtr ptr);
//
STScnVertexIdxPtr ScnVertexbuffs_v0IdxsAlloc(STScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v0IdxsInvalidate(STScnVertexbuffsRef ref, const STScnVertexPtr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v0IdxsFree(STScnVertexbuffsRef ref, const STScnVertexPtr ptr);

//ENScnVertexType_Tex  //one texture

STScnVertexTexPtr ScnVertexbuffs_v1Alloc(STScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v1Invalidate(STScnVertexbuffsRef ref, const STScnVertexPtr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v1Free(STScnVertexbuffsRef ref, const STScnVertexPtr ptr);
//
STScnVertexIdxPtr ScnVertexbuffs_v1IdxsAlloc(STScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v1IdxsInvalidate(STScnVertexbuffsRef ref, const STScnVertexPtr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v1IdxsFree(STScnVertexbuffsRef ref, const STScnVertexPtr ptr);

//ENScnVertexType_Tex2 //two textures

STScnVertexTex2Ptr ScnVertexbuffs_v2Alloc(STScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v2Invalidate(STScnVertexbuffsRef ref, const STScnVertexPtr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v2Free(STScnVertexbuffsRef ref, const STScnVertexPtr ptr);
//
STScnVertexIdxPtr ScnVertexbuffs_v2IdxsAlloc(STScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v2IdxsInvalidate(STScnVertexbuffsRef ref, const STScnVertexPtr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v2IdxsFree(STScnVertexbuffsRef ref, const STScnVertexPtr ptr);

//ENScnVertexType_Tex3 //three textures

STScnVertexTex3Ptr ScnVertexbuffs_v3Alloc(STScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v3Invalidate(STScnVertexbuffsRef ref, const STScnVertexPtr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v3Free(STScnVertexbuffsRef ref, const STScnVertexPtr ptr);
//
STScnVertexIdxPtr ScnVertexbuffs_v3IdxsAlloc(STScnVertexbuffsRef ref, const ScnUI32 amm);
ScnBOOL         ScnVertexbuffs_v3IdxsInvalidate(STScnVertexbuffsRef ref, const STScnVertexPtr ptr, const ScnUI32 sz);
ScnBOOL         ScnVertexbuffs_v3IdxsFree(STScnVertexbuffsRef ref, const STScnVertexPtr ptr);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnVertexbuff_h */
