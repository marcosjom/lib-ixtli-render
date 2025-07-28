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

#define STScnGpuBufferCfg_Zero   { ENScnVertexType_Color, STScnMemElasticCfg_Zero }

typedef struct STScnGpuBufferCfg_ {
    ENScnVertexType     type;
    STScnMemElasticCfg  mem;    //memory blocks cfg
} STScnGpuBufferCfg;

//STScnGpuBufferChanges

typedef struct STScnGpuBufferChanges_ {
    ScnBOOL         size;  //buffer's size changed
    STScnRangeU*    rngs;  //subimages areas
    ScnUI32         rngsUse;
} STScnGpuBufferChanges;

//STScnGpuBufferApiItf

typedef struct STScnGpuBufferApiItf_ {
    void*   (*create)(const STScnGpuBufferCfg* cfg, void* usrData);
    void    (*destroy)(void* data, void* usrData);
    //
    ScnBOOL (*sync)(void* data, const STScnGpuBufferCfg* cfg, STScnMemElasticRef mem, const STScnGpuBufferChanges* changes, void* usrData);
} STScnGpuBufferApiItf;

//STScnGpuBufferRef

SCN_REF_STRUCT_METHODS_DEC(ScnGpuBuffer)

//

ScnBOOL     ScnGpuBuffer_prepare(STScnGpuBufferRef ref, const STScnGpuBufferCfg* cfg, const STScnGpuBufferApiItf* itf, void* itfParam);
ScnBOOL     ScnGpuBuffer_clear(STScnGpuBufferRef ref);

STScnAbsPtr ScnGpuBuffer_malloc(STScnGpuBufferRef ref, const ScnUI32 usableSz);
ScnBOOL     ScnGpuBuffer_mfree(STScnGpuBufferRef ref, const STScnAbsPtr ptr);
ScnBOOL     ScnGpuBuffer_mInvalidate(STScnGpuBufferRef ref, const STScnAbsPtr ptr, const ScnUI32 sz);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuBuffer_h */
