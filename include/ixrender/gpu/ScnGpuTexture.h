//
//  ScnGpuTexture.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnGpuTexture_h
#define ScnGpuTexture_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/type/ScnPoint.h"
#include "ixrender/type/ScnRect.h"
#include "ixrender/type/ScnRange.h"
#include "ixrender/type/ScnBitmap.h"
#include "ixrender/gpu/ScnGpuSampler.h"

#ifdef __cplusplus
extern "C" {
#endif

//ENScnGpuTextureIdx

typedef enum ENScnGpuTextureIdx {
    ENScnGpuTextureIdx_0 = 0,
    ENScnGpuTextureIdx_1,
    ENScnGpuTextureIdx_2,
    //Count
    ENScnGpuTextureIdx_Count
} ENScnGpuTextureIdx;

//STScnGpuTextureCfg

#define STScnGpuTextureCfg_Zero   { ENScnBitmapColor_undef, 0, 0, ScnFALSE, STScnGpuSamplerCfg_Zero }

typedef struct STScnGpuTextureCfg {
    ENScnBitmapColor    color;
    ScnUI32             width;
    ScnUI32             height;
    ScnBOOL             mipmapEnabled;
    STScnGpuSamplerCfg  sampler;
} STScnGpuTextureCfg;

//STScnGpuTextureChanges

#define STScnGpuTextureChanges_Zero {  ScnFALSE, NULL, 0 }

typedef struct STScnGpuTextureChanges {
    ScnBOOL         all;    //all the content is new
    STScnRangeU*    rngs;   //rngs changed (in full-lines)
    ScnUI32         rngsUse;
} STScnGpuTextureChanges;

//STScnGpuTextureApiItf

/** @struct STScnGpuTextureApiItf
 *  @brief Texture's API interface.
 *  @var STScnGpuTextureApiItf::free
 *  Method to free the texture.
 *  @var STScnGpuTextureApiItf::sync
 *  Method to synchronize the texture's data.
 */
typedef struct STScnGpuTextureApiItf {
    void        (*free)(void* data);
    //
    ScnBOOL     (*sync)(void* data, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData, const STScnGpuTextureChanges* const changes);
} STScnGpuTextureApiItf;

//ScnGpuTextureRef

/** @struct ScnGpuTextureRef
 *  @brief ScnGpuTexture shared pointer. An abstract object based on the currently used API.
 */

#define ScnGpuTextureRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnGpuTexture)

//

/**
 * @brief Prepares the gpu abstract object with the provided interface.
 * @param ref Reference to object.
 * @param itf Interface to the API.
 * @param itfParam Parameter to be given to the interface's  methods.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnGpuTexture_prepare(ScnGpuTextureRef ref, const STScnGpuTextureApiItf* itf, void* itfParam);

/**
 * @brief Retrieves the gpu abstract object pointer.
 * @param ref Reference to object.
 * @return Abstract object's pointer on success, NULL otherwise.
 */
void* ScnGpuTexture_getApiItfParam(ScnGpuTextureRef ref);

/**
 * @brief Synchronizes the texture's data.
 * @param ref Reference to object.
 * @param cfg The texture's configuration.
 * @param srcProps The data's properties.
 * @param srcData The data's buffer.
 * @param changes The changes's to be synchronized.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnGpuTexture_sync(ScnGpuTextureRef ref, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData, const STScnGpuTextureChanges* const changes);


#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuTexture_h */
