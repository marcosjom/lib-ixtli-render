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

typedef enum ENScnGpusamplerAddress {
    ENScnGpusamplerAddress_Repeat = 0, //pattern
    ENScnGpusamplerAddress_Clamp,      //single-image
    //
    ENScnGpusamplerAddress_Count
} ENScnGpusamplerAddress;

//ENScnGpuSamplerFilter

typedef enum ENScnGpuSamplerFilter {
    ENScnGpuSamplerFilter_Nearest = 0, //fast selection of nearest color
    ENScnGpuSamplerFilter_Linear,      //calculation of merged color
    //
    ENScnGpuSamplerFilter_Count
} ENScnGpuSamplerFilter;

//STScnGpuSamplerCfg

#define STScnGpuSamplerCfg_Zero   { ENScnGpusamplerAddress_Repeat, ENScnGpuSamplerFilter_Nearest, ENScnGpuSamplerFilter_Nearest }

typedef struct STScnGpuSamplerCfg {
    ENScnGpusamplerAddress  address;
    ENScnGpuSamplerFilter   magFilter;
    ENScnGpuSamplerFilter   minFilter;
} STScnGpuSamplerCfg;

//STScnGpuSamplerApiItf

typedef struct STScnGpuSamplerApiItf {
    void        (*free)(void* data);
    //
    STScnGpuSamplerCfg (*getCfg)(void* data);
} STScnGpuSamplerApiItf;

//ScnGpuSamplerRef

#define ScnGpuSamplerRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnGpuSampler)

//

ScnBOOL             ScnGpuSampler_prepare(ScnGpuSamplerRef ref, const STScnGpuSamplerApiItf* itf, void* itfParam);
void*               ScnGpuSampler_getApiItfParam(ScnGpuSamplerRef ref);
//
STScnGpuSamplerCfg  ScnGpuSampler_getCfg(ScnGpuSamplerRef ref);


#ifdef __cplusplus
} //extern "C"
#endif


#endif /* ScnGpuSampler_h */
