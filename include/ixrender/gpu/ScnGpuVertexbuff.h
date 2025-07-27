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

typedef struct STScnGpuVertexPartDef_ {
    ScnUI8  amm;        //ammount of elements of this part (coords, color, textureCoords, ...)
    ScnUI8  type;       //ENScnGpuDataType
    ScnUI16 offset;     //offset inside a record to the first element
} STScnGpuVertexPartDef;

//STScnGpuVertexbuffCfg

#define STScnGpuVertexbuffCfg_Zero   { 0, STScnGpuVertexPartDef_Zero, STScnGpuVertexPartDef_Zero, STScnGpuVertexPartDef_Zero, { STScnGpuVertexPartDef_Zero, STScnGpuVertexPartDef_Zero, STScnGpuVertexPartDef_Zero } }

typedef struct STScnGpuVertexbuffCfg_ {
    ScnUI32                 szPerRecord;    //bytes per record
    STScnGpuVertexPartDef   indices;
    STScnGpuVertexPartDef   coord;
    STScnGpuVertexPartDef   color;
    STScnGpuVertexPartDef   texCoords[ENScnGpuTextureIdx_Count];
} STScnGpuVertexbuffCfg;

//STScnGpuVertexbuffApiItf

typedef struct STScnGpuVertexbuffApiItf_ {
    void* (*create)(const STScnGpuVertexbuffCfg* cfg, STScnGpuBufferRef vertexBuff, STScnGpuBufferRef idxsBuff, void* usrData);
    void  (*destroy)(void* data, void* usrData);
    //
    ScnBOOL  (*activate)(void* data, const STScnGpuVertexbuffCfg* cfg, void* usrData);
    ScnBOOL  (*deactivate)(void* data, void* usrData);
} STScnGpuVertexbuffApiItf;

//

//STScnGpuVertexbuffRef

SCN_REF_STRUCT_METHODS_DEC(ScnGpuVertexbuff)

//

ScnBOOL                ScnGpuVertexbuff_prepare(STScnGpuVertexbuffRef ref, const STScnGpuVertexbuffCfg* cfg, STScnGpuBufferRef vertexBuff, STScnGpuBufferRef idxsBuff, const STScnGpuVertexbuffApiItf* itf, void* itfParam);

ScnBOOL                ScnGpuVertexbuff_activate(STScnGpuVertexbuffRef ref);
ScnBOOL                ScnGpuVertexbuff_deactivate(STScnGpuVertexbuffRef ref);

ScnUI32                ScnGpuVertexbuff_getSzPerRecord(STScnGpuVertexbuffRef ref);
STScnGpuBufferRef    ScnGpuVertexbuff_getVertexBuff(STScnGpuVertexbuffRef ref);
STScnGpuBufferRef    ScnGpuVertexbuff_getIdxsBuff(STScnGpuVertexbuffRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuVertexbuff_h */
