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

#define STScnGpuVertexbuffCfg_Zero   { 0, STScnGpuVertexPartDef_Zero, STScnGpuVertexPartDef_Zero, STScnGpuVertexPartDef_Zero, { STScnGpuVertexPartDef_Zero, STScnGpuVertexPartDef_Zero, STScnGpuVertexPartDef_Zero } }

typedef struct STScnGpuVertexbuffCfg {
    ScnUI32                 szPerRecord;    //bytes per record
    STScnGpuVertexPartDef   indices;
    STScnGpuVertexPartDef   coord;
    STScnGpuVertexPartDef   color;
    STScnGpuVertexPartDef   texCoords[ENScnGpuTextureIdx_Count];
} STScnGpuVertexbuffCfg;

//STScnGpuVertexbuffApiItf

typedef struct STScnGpuVertexbuffApiItf {
    void    (*free)(void* data);
    //
    ScnBOOL (*sync)(void* data, const STScnGpuVertexbuffCfg* cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff);
    ScnBOOL (*activate)(void* data);
    ScnBOOL (*deactivate)(void* data);
} STScnGpuVertexbuffApiItf;

//

//ScnGpuVertexbuffRef

SCN_REF_STRUCT_METHODS_DEC(ScnGpuVertexbuff)

//

ScnBOOL             ScnGpuVertexbuff_prepare(ScnGpuVertexbuffRef ref, const STScnGpuVertexbuffApiItf* itf, void* itfParam);

ScnBOOL             ScnGpuVertexbuff_sync(ScnGpuVertexbuffRef ref, const STScnGpuVertexbuffCfg* cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff);
ScnBOOL             ScnGpuVertexbuff_activate(ScnGpuVertexbuffRef ref);
ScnBOOL             ScnGpuVertexbuff_deactivate(ScnGpuVertexbuffRef ref);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuVertexbuff_h */
