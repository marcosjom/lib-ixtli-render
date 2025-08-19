//
//  ScnGpuVertexbuff.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnGpuVertexbuff_h
#define ScnGpuVertexbuff_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/gpu/ScnGpuTexture.h"
#include "ixrender/gpu/ScnGpuBuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnGpuVertexPartDef

#define STScnGpuVertexPartDef_Zero    { 0, 0, 0 }

typedef struct STScnGpuVertexPartDef {
    ScnUI8  amm;        //ammount of elements of this part (coords, color, textureCoords, ...)
    ScnUI8  type;       //ENScnGpuDataType
    ScnUI16 offset;     //offset inside a record to the first element
} STScnGpuVertexPartDef;

//STScnGpuVertexbuffCfg

#define STScnGpuVertexbuffCfg_Zero   { 0, STScnGpuVertexPartDef_Zero, STScnGpuVertexPartDef_Zero, { STScnGpuVertexPartDef_Zero, STScnGpuVertexPartDef_Zero, STScnGpuVertexPartDef_Zero } }

#define STScnGpuVertexbuffCfg_2_ENScnVertexType(OBJ)  ((OBJ)->texCoords[ENScnGpuTextureIdx_2].amm > 0 ? ENScnVertexType_2DTex3 : (OBJ)->texCoords[ENScnGpuTextureIdx_1].amm > 0 ? ENScnVertexType_2DTex2 : (OBJ)->texCoords[ENScnGpuTextureIdx_0].amm > 0 ? ENScnVertexType_2DTex : ENScnVertexType_2DColor)

typedef struct STScnGpuVertexbuffCfg {
    ScnUI32                 szPerRecord;    //bytes per record
    STScnGpuVertexPartDef   coord;
    STScnGpuVertexPartDef   color;
    STScnGpuVertexPartDef   texCoords[ENScnGpuTextureIdx_Count];
} STScnGpuVertexbuffCfg;

//STScnGpuVertexbuffApiItf

/** @struct STScnGpuVertexbuffApiItf
 *  @brief Vertex buffer's API interface.
 *  @var STScnGpuVertexbuffApiItf::free
 *  Method to free the buffer.
 *  @var STScnGpuVertexbuffApiItf::sync
 *  Method to synchronize the buffer's data.
 *  @var STScnGpuVertexbuffApiItf::activate
 *  Method to bind the buffers to the render job.
 *  @var STScnGpuVertexbuffApiItf::deactivate
 *  Method to unbind the buffers to the render job.
 */
typedef struct STScnGpuVertexbuffApiItf {
    void    (*free)(void* data);
    //
    ScnBOOL (*sync)(void* data, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff);
    ScnBOOL (*activate)(void* data);
    ScnBOOL (*deactivate)(void* data);
} STScnGpuVertexbuffApiItf;

//

//ScnGpuVertexbuffRef

/** @struct ScnGpuVertexbuffRef
 *  @brief ScnGpuVertexbuff shared pointer. An abstract object based on the currently used API.
 */

#define ScnGpuVertexbuffRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnGpuVertexbuff)

//

/**
 * @brief Prepares the gpu abstract object with the provided interface.
 * @param ref Reference to object.
 * @param itf Interface to the API.
 * @param itfParam Parameter to be given to the interface's  methods.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnGpuVertexbuff_prepare(ScnGpuVertexbuffRef ref, const STScnGpuVertexbuffApiItf* itf, void* itfParam);

/**
 * @brief Retrieves the gpu abstract object pointer.
 * @param ref Reference to object.
 * @return Abstract object's pointer on success, NULL otherwise.
 */
void* ScnGpuVertexBuff_getApiItfParam(ScnGpuVertexbuffRef ref);

/**
 * @brief Synchronizes the vertex buffer's data.
 * @param ref Reference to object.
 * @param cfg The vertex buffer's configuration.
 * @param vBuff The vertices buffer.
 * @param idxBuff The indices buffer.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnGpuVertexbuff_sync(ScnGpuVertexbuffRef ref, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff);

/**
 * @brief Binds the vertex buffer.
 * @param ref Reference to object.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnGpuVertexbuff_activate(ScnGpuVertexbuffRef ref);

/**
 * @brief Unbinds the vertex buffer.
 * @param ref Reference to object.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnGpuVertexbuff_deactivate(ScnGpuVertexbuffRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuVertexbuff_h */
