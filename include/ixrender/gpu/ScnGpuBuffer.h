//
//  ScnGpuBuffer.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnGpuBuffer_h
#define ScnGpuBuffer_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/core/ScnMemElastic.h"
#include "ixrender/type/ScnRange.h"
#include "ixrender/scene/ScnVertices.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnGpuBufferCfg

#define STScnGpuBufferCfg_Zero   { STScnMemElasticCfg_Zero }

typedef struct STScnGpuBufferCfg_ {
    STScnMemElasticCfg  mem;    //memory blocks cfg
} STScnGpuBufferCfg;

//STScnGpuBufferChanges

#define STScnGpuBufferChanges_Zero { ScnFALSE, NULL, 0 }

typedef struct STScnGpuBufferChanges_ {
    ScnBOOL         size;  //buffer's size changed
    STScnRangeU*    rngs;  //rngs changed
    ScnUI32         rngsUse;
} STScnGpuBufferChanges;

//STScnGpuBufferApiItf

typedef struct STScnGpuBufferApiItf_ {
    void    (*free)(void* data);
    ScnBOOL (*sync)(void* data, const STScnGpuBufferCfg* cfg, STScnMemElasticRef mem, const STScnGpuBufferChanges* changes);
} STScnGpuBufferApiItf;

//STScnGpuBufferRef

SCN_REF_STRUCT_METHODS_DEC(ScnGpuBuffer)

//

ScnBOOL ScnGpuBuffer_prepare(STScnGpuBufferRef ref, const STScnGpuBufferApiItf* itf, void* itfParam);
ScnBOOL ScnGpuBuffer_sync(STScnGpuBufferRef ref, const STScnGpuBufferCfg* cfg, STScnMemElasticRef mem, const STScnGpuBufferChanges* changes);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuBuffer_h */
