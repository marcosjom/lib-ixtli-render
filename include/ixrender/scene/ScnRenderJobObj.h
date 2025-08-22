//
//  ScnRenderJobObj.h
//  ixtli-render
//
//  Created by Marcos Ortega on 22/8/25.
//

#ifndef ScnRenderJobObj_h
#define ScnRenderJobObj_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/scene/ScnBuffer.h"
#include "ixrender/scene/ScnFramebuff.h"
#include "ixrender/scene/ScnVertexbuff.h"
#include "ixrender/scene/ScnTexture.h"

#ifdef __cplusplus
extern "C" {
#endif

//ENScnRenderJobObjType

/** @struct ENScnRenderJobObjType
 *  @brief Commands container object's type.
 */

typedef enum ENScnRenderJobObjType {
    ENScnRenderJobObjType_Unknown = 0,
    ENScnRenderJobObjType_Buff,
    ENScnRenderJobObjType_Framebuff,
    ENScnRenderJobObjType_Vertexbuff,
    ENScnRenderJobObjType_Texture,
    //
    ENScnRenderJobObjType_Count
} ENScnRenderJobObjType;

//STScnRenderJobObj

/** @struct STScnRenderJobObj
 *  @brief Record of a object used in a commands container.
 *  @var STScnRenderJobObj::type
 *  Objects type
 */

typedef struct STScnRenderJobObj {
    ENScnRenderJobObjType   type;
    union {
        ScnObjRef           objRef;     //generic ref (compatible will all bellow it)
        ScnBufferRef        buff;
        ScnFramebuffRef     framebuff;
        ScnVertexbuffRef    vertexbuff;
        ScnTextureRef       texture;
    };
} STScnRenderJobObj;

/**
 * @brief Initializes the record.
 * @param obj Reference to object.
 */
void ScnRenderJobObj_init(STScnRenderJobObj* obj);

/**
 * @brief Destroys the record.
 * @param obj Reference to object.
 */
void ScnRenderJobObj_destroy(STScnRenderJobObj* obj);

/**
 * @brief Compares tow records.
 * @param data1 Pointer to first record.
 * @param data2 Pointer to second record.
 * @param dataSz Size of the data referenced by both pointers.
 * @return Zero if both are equal, a positive value if data1 is greater than data2, a negative value otherwise.
 */

ScnSI32 ScnCompare_ScnRenderJobObj(const void* data1, const void* data2, const ScnUI32 dataSz);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRenderJobObj_h */
