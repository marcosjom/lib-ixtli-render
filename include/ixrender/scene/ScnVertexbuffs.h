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
#include "ixrender/scene/ScnVertex.h"
#include "ixrender/scene/ScnVertexbuff.h"


#ifdef __cplusplus
extern "C" {
#endif

//STScnVertexIdxPtr, abstract pointer

/** @struct STScnVertexIdxPtr
 *  @brief Vertex index pointer.
 *  @var STScnVertexIdxPtr::ptr
 *  Pointer to indices.
 *  @var STScnVertexIdxPtr::idx
 *  Pointer's index inside the buffer, in indices size.
 */

#define STScnVertexIdxPtr_Zero { NULL, 0 }

typedef struct STScnVertexIdxPtr {
    STScnVertexIdx*     ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    ScnUI32             idx;    //abstract address
} STScnVertexIdxPtr;

//STScnVertex2DPtr, abstract pointer

/** @struct STScnVertex2DPtr
 *  @brief Vertex pointer.
 *  @var STScnVertex2DPtr::ptr
 *  Pointer to vertices.
 *  @var STScnVertex2DPtr::idx
 *  Pointer's index inside the buffer, in vertex size.
 */

#define STScnVertex2DPtr_Zero { NULL, 0 }

typedef struct STScnVertex2DPtr {
    STScnVertex2D*      ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    ScnUI32             idx;    //abstract address
} STScnVertex2DPtr;

//STScnVertex2DTexPtr, abstract pointer

/** @struct STScnVertex2DTexPtr
 *  @brief Vertex pointer.
 *  @var STScnVertex2DTexPtr::ptr
 *  Pointer to vertices.
 *  @var STScnVertex2DTexPtr::idx
 *  Pointer's index inside the buffer, in vertex size.
 */

#define STScnVertex2DTexPtr_Zero { NULL, 0 }

typedef struct STScnVertex2DTexPtr {
    STScnVertex2DTex*   ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    ScnUI32             idx;    //abstract address
} STScnVertex2DTexPtr;

//STScnVertex2DTex2Ptr, abstract pointer

/** @struct STScnVertex2DTex2Ptr
 *  @brief Vertex pointer.
 *  @var STScnVertex2DTex2Ptr::ptr
 *  Pointer to vertices.
 *  @var STScnVertex2DTex2Ptr::idx
 *  Pointer's index inside the buffer, in vertex size.
 */

#define STScnVertex2DTex2Ptr_Zero { NULL, 0 }

typedef struct STScnVertex2DTex2Ptr {
    STScnVertex2DTex2*  ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    ScnUI32             idx;    //abstract address
} STScnVertex2DTex2Ptr;

//STScnVertex2DTex3Ptr, abstract pointer

/** @struct STScnVertex2DTex3Ptr
 *  @brief Vertex pointer.
 *  @var STScnVertex2DTex3Ptr::ptr
 *  Pointer to vertices.
 *  @var STScnVertex2DTex3Ptr::idx
 *  Pointer's index inside the buffer, in vertex size.
 */

#define STScnVertex2DTex3Ptr_Zero { NULL, 0 }

typedef struct STScnVertex2DTex3Ptr {
    STScnVertex2DTex3*  ptr;    //memory address, must be first element of struct to allow casting struct to a bare-pointer.
    ScnUI32             idx;    //abstract address
} STScnVertex2DTex3Ptr;

//

//ScnVertexbuffsRef

/** @struct ScnVertexbuffsRef
 *  @brief ScnVertexbuffs shared pointer. An object that contains multiple vertexbuffers, for each type of vertex and their indices.
 */

#define ScnVertexbuffsRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnVertexbuffs)

//Prepare

/**
 * @brief Prepares the abstract vertexbuffers with the provided vertexbuffer array; containing one cpu-data slot and one or multiple gpu-data slots.
 * @param ref Reference to object.
 * @param vBuffs Pointer to a vertexbuffer array.
 * @param vBuffsSz Lenght of the vertexbuffer array, as items count.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuffs_prepare(ScnVertexbuffsRef ref, const ScnVertexbuffRef* vBuffs, const ScnUI32 vBuffsSz);

/**
 * @brief Retrieves the vertexbuffer for the specified type of vertex.
 * @param ref Reference to object.
 * @param type Type of vertex.
 * @return Vertexbuffer's reference on success, ScnVertexbuffRef_Zero otherwise.
 */
ScnVertexbuffRef ScnVertexbuffs_getVertexBuff(ScnVertexbuffsRef ref, const ENScnVertexType type);

//ENScnVertexType_2DColor //no texture

/**
 * @brief Allocates vertices without texture.
 * @param ref Reference to object.
 * @param amm Ammount of vertices.
 * @return Pointer to the vertices allocated on success, STScnVertex2DPtr_Zero otherwise.
 */
STScnVertex2DPtr ScnVertexbuffs_v0Alloc(ScnVertexbuffsRef ref, const ScnUI32 amm);

/**
 * @brief Flags the specified range of previously allocated vertices without texture to be synced in to the gpu-slots.
 * @param ref Reference to object.
 * @param ptr Pointer to vertices.
 * @param sz Ammount of vertices.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuffs_v0Invalidate(ScnVertexbuffsRef ref, const STScnVertex2DPtr ptr, const ScnUI32 sz);

/**
 * @brief Frees a previously allocated vertices without texture.
 * @param ref Reference to object.
 * @param ptr Pointer to vertices.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuffs_v0Free(ScnVertexbuffsRef ref, const STScnVertex2DPtr ptr);

//

/**
 * @brief Allocates vertex without texture indices.
 * @param ref Reference to object.
 * @param amm Ammount of indices.
 * @return Pointer to the indices allocated on success, STScnVertexIdxPtr_Zero otherwise.
 */
STScnVertexIdxPtr ScnVertexbuffs_v0IdxsAlloc(ScnVertexbuffsRef ref, const ScnUI32 amm);

/**
 * @brief Flags the specified range of previously allocated indices without texture to be synced in to the gpu-slots.
 * @param ref Reference to object.
 * @param ptr Pointer to indices.
 * @param sz Ammount of indices.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuffs_v0IdxsInvalidate(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr, const ScnUI32 sz);

/**
 * @brief Frees a previously allocated indices without texture.
 * @param ref Reference to object.
 * @param ptr Pointer to indices.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuffs_v0IdxsFree(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr);

//ENScnVertexType_2DTex  //one texture

/**
 * @brief Allocates vertices with one texture.
 * @param ref Reference to object.
 * @param amm Ammount of vertices.
 * @return Pointer to the vertices allocated on success, STScnVertex2DTexPtr_Zero otherwise.
 */
STScnVertex2DTexPtr ScnVertexbuffs_v1Alloc(ScnVertexbuffsRef ref, const ScnUI32 amm);

/**
 * @brief Flags the specified range of previously allocated vertices with one texture to be synced in to the gpu-slots.
 * @param ref Reference to object.
 * @param ptr Pointer to vertices.
 * @param sz Ammount of vertices.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuffs_v1Invalidate(ScnVertexbuffsRef ref, const STScnVertex2DTexPtr ptr, const ScnUI32 sz);

/**
 * @brief Frees a previously allocated vertices with one texture.
 * @param ref Reference to object.
 * @param ptr Pointer to vertices.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuffs_v1Free(ScnVertexbuffsRef ref, const STScnVertex2DTexPtr ptr);

//

/**
 * @brief Allocates vertex with one texture indices.
 * @param ref Reference to object.
 * @param amm Ammount of indices.
 * @return Pointer to the indices allocated on success, STScnVertexIdxPtr_Zero otherwise.
 */
STScnVertexIdxPtr ScnVertexbuffs_v1IdxsAlloc(ScnVertexbuffsRef ref, const ScnUI32 amm);

/**
 * @brief Flags the specified range of previously allocated indices with one texture to be synced in to the gpu-slots.
 * @param ref Reference to object.
 * @param ptr Pointer to indices.
 * @param sz Ammount of indices.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuffs_v1IdxsInvalidate(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr, const ScnUI32 sz);

/**
 * @brief Frees a previously allocated indices with one texture.
 * @param ref Reference to object.
 * @param ptr Pointer to indices.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuffs_v1IdxsFree(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr);

//ENScnVertexType_2DTex2 //two textures

/**
 * @brief Allocates vertices with two textures.
 * @param ref Reference to object.
 * @param amm Ammount of vertices.
 * @return Pointer to the vertices allocated on success, STScnVertex2DTex2Ptr_Zero otherwise.
 */
STScnVertex2DTex2Ptr ScnVertexbuffs_v2Alloc(ScnVertexbuffsRef ref, const ScnUI32 amm);

/**
 * @brief Flags the specified range of previously allocated vertices with two textures to be synced in to the gpu-slots.
 * @param ref Reference to object.
 * @param ptr Pointer to vertices.
 * @param sz Ammount of vertices.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuffs_v2Invalidate(ScnVertexbuffsRef ref, const STScnVertex2DTex2Ptr ptr, const ScnUI32 sz);

/**
 * @brief Frees a previously allocated vertices with two textures.
 * @param ref Reference to object.
 * @param ptr Pointer to vertices.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuffs_v2Free(ScnVertexbuffsRef ref, const STScnVertex2DTex2Ptr ptr);

//

/**
 * @brief Allocates vertex with two textures indices.
 * @param ref Reference to object.
 * @param amm Ammount of indices.
 * @return Pointer to the indices allocated on success, STScnVertexIdxPtr_Zero otherwise.
 */
STScnVertexIdxPtr ScnVertexbuffs_v2IdxsAlloc(ScnVertexbuffsRef ref, const ScnUI32 amm);

/**
 * @brief Flags the specified range of previously allocated indices with two textures to be synced in to the gpu-slots.
 * @param ref Reference to object.
 * @param ptr Pointer to indices.
 * @param sz Ammount of indices.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuffs_v2IdxsInvalidate(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr, const ScnUI32 sz);

/**
 * @brief Frees a previously allocated indices with two textures.
 * @param ref Reference to object.
 * @param ptr Pointer to indices.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuffs_v2IdxsFree(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr);

//ENScnVertexType_2DTex3 //three textures

/**
 * @brief Allocates vertices with three textures.
 * @param ref Reference to object.
 * @param amm Ammount of vertices.
 * @return Pointer to the vertices allocated on success, STScnVertex2DTex3Ptr_Zero  otherwise.
 */
STScnVertex2DTex3Ptr ScnVertexbuffs_v3Alloc(ScnVertexbuffsRef ref, const ScnUI32 amm);

/**
 * @brief Flags the specified range of previously allocated vertices with three textures to be synced in to the gpu-slots.
 * @param ref Reference to object.
 * @param ptr Pointer to vertices.
 * @param sz Ammount of vertices.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuffs_v3Invalidate(ScnVertexbuffsRef ref, const STScnVertex2DTex3Ptr ptr, const ScnUI32 sz);

/**
 * @brief Frees a previously allocated vertices with three textures.
 * @param ref Reference to object.
 * @param ptr Pointer to vertices.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuffs_v3Free(ScnVertexbuffsRef ref, const STScnVertex2DTex3Ptr ptr);

//

/**
 * @brief Allocates vertex with three textures indices.
 * @param ref Reference to object.
 * @param amm Ammount of indices.
 * @return Pointer to the indices allocated on success, STScnVertexIdxPtr_Zero otherwise.
 */
STScnVertexIdxPtr ScnVertexbuffs_v3IdxsAlloc(ScnVertexbuffsRef ref, const ScnUI32 amm);

/**
 * @brief Flags the specified range of previously allocated indices with three textures to be synced in to the gpu-slots.
 * @param ref Reference to object.
 * @param ptr Pointer to indices.
 * @param sz Ammount of indices.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuffs_v3IdxsInvalidate(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr, const ScnUI32 sz);

/**
 * @brief Frees a previously allocated indices with three textures.
 * @param ref Reference to object.
 * @param ptr Pointer to indices.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuffs_v3IdxsFree(ScnVertexbuffsRef ref, const STScnVertexIdxPtr ptr);

//gpu-vertexbuffs

/**
 * @brief Prepares the current render slot for gpu commands execution. The slot's gpu-data is synchronized with the current cpu-data.
 * @param ref Reference to object.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuffs_prepareCurrentRenderSlot(ScnVertexbuffsRef ref);

/**
 * @brief Moves the index to the next render slot for future gpu commands execution.
 * @param ref Reference to object.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnVertexbuffs_moveToNextRenderSlot(ScnVertexbuffsRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnVertexbuff_h */
