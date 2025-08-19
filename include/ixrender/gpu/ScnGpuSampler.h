//
//  ScnGpuSampler.h
//  nbframework
//
//  Created by Marcos Ortega on 8/8/25.
//

#ifndef ScnGpuSampler_h
#define ScnGpuSampler_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"

#ifdef __cplusplus
extern "C" {
#endif

//ENScnGpusamplerAddress

/** @enum ENScnGpusamplerAddress
 *  @brief Samplers address modes.
 */

typedef enum ENScnGpusamplerAddress {
    ENScnGpusamplerAddress_Repeat = 0, //pattern
    ENScnGpusamplerAddress_Clamp,      //single-image
    //
    ENScnGpusamplerAddress_Count
} ENScnGpusamplerAddress;

//ENScnGpuSamplerFilter

/** @enum ENScnGpuSamplerFilter
 *  @brief Samplers filters modes.
 */

typedef enum ENScnGpuSamplerFilter {
    ENScnGpuSamplerFilter_Nearest = 0, //fast selection of nearest color
    ENScnGpuSamplerFilter_Linear,      //calculation of merged color
    //
    ENScnGpuSamplerFilter_Count
} ENScnGpuSamplerFilter;

//STScnGpuSamplerCfg

/** @struct STScnGpuSamplerCfg
 *  @brief Sampler configuration.
 *  @var STScnGpuSamplerCfg::address
 *  Address mode.
 *  @var STScnGpuSamplerCfg::magFilter
 *  Magnifier filter.
 *  @var STScnGpuSamplerCfg::minFilter
 *  Minifier filter.
 */

#define STScnGpuSamplerCfg_Zero   { ENScnGpusamplerAddress_Repeat, ENScnGpuSamplerFilter_Nearest, ENScnGpuSamplerFilter_Nearest }

typedef struct STScnGpuSamplerCfg {
    ENScnGpusamplerAddress  address;
    ENScnGpuSamplerFilter   magFilter;
    ENScnGpuSamplerFilter   minFilter;
} STScnGpuSamplerCfg;

//STScnGpuSamplerApiItf

/** @struct STScnGpuSamplerApiItf
 *  @brief Sampler's API interface.
 *  @var STScnGpuSamplerApiItf::free
 *  Method to free the sampler.
 *  @var STScnGpuSamplerApiItf::getCfg
 *  Method to retrieve the configuration of the sampler.
 */
typedef struct STScnGpuSamplerApiItf {
    void        (*free)(void* data);
    //
    STScnGpuSamplerCfg (*getCfg)(void* data);
} STScnGpuSamplerApiItf;

//ScnGpuSamplerRef

/** @struct ScnGpuSamplerRef
 *  @brief ScnGpuSampler shared pointer. An abstract object based on the currently used API.
 */

#define ScnGpuSamplerRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnGpuSampler)

//

/**
 * @brief Prepares the gpu abstract object with the provided interface.
 * @param ref Reference to object.
 * @param itf Interface to the API.
 * @param itfParam Parameter to be given to the interface's  methods.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnGpuSampler_prepare(ScnGpuSamplerRef ref, const STScnGpuSamplerApiItf* itf, void* itfParam);

/**
 * @brief Retrieves the gpu abstract object pointer.
 * @param ref Reference to object.
 * @return Abstract object's pointer on success, NULL otherwise.
 */
void* ScnGpuSampler_getApiItfParam(ScnGpuSamplerRef ref);

/**
 * @brief Retrieves the sampler configuration.
 * @param ref Reference to object.
 * @return The configuration on success, STScnGpuSamplerCfg_Zero otherwise.
 */
STScnGpuSamplerCfg ScnGpuSampler_getCfg(ScnGpuSamplerRef ref);


#ifdef __cplusplus
} //extern "C"
#endif


#endif /* ScnGpuSampler_h */
