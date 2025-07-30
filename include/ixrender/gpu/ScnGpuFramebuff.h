//
//  ScnGpuFramebuff.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnGpuFramebuff_h
#define ScnGpuFramebuff_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/gpu/ScnGpuFramebuffProps.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnGpuFramebuffCfg

#define STScnGpuFramebuffCfg_Zero   { ENScnBitmapColor_undef, 0, 0 }

typedef struct STScnGpuFramebuffCfg_ {
    STScnGpuFramebufferProps props;
} STScnGpuFramebuffCfg;

//ENScnGpuFramebuffDstType

typedef enum ENScnGpuFramebuffDstType_ {
    ENScnGpuFramebuffDstType_None = 0,
    ENScnGpuFramebuffDstType_Texture,
    ENScnGpuFramebuffDstType_Renderbuffer,
    //Count
    ENScnGpuFramebuffDstType_Count
} ENScnGpuFramebuffDstType;

//STScnGpuFramebuffChanges

typedef struct STScnGpuFramebuffChanges_ {
    ScnBOOL    bind;
} STScnGpuFramebuffChanges;

//STScnGpuFramebuffApiItf

typedef struct STScnGpuFramebuffApiItf_ {
    void*   (*create)(const STScnGpuFramebuffCfg* cfg, void* usrData);
    void    (*destroy)(void* data, void* usrData);
    //
    ScnBOOL (*sync)(void* data, const STScnGpuFramebuffCfg* cfg, const STScnGpuFramebuffChanges* changes, void* usrData);
} STScnGpuFramebuffApiItf;


//ScnGpuFramebuffRef

SCN_REF_STRUCT_METHODS_DEC(ScnGpuFramebuff)

//

ScnBOOL ScnGpuFramebuff_prepare(ScnGpuFramebuffRef ref, const STScnGpuFramebuffCfg* cfg, const STScnGpuFramebuffApiItf* itf, void* itfParam);
ScnBOOL ScnGpuFramebuff_bindTo(ScnGpuFramebuffRef ref, const ScnObjRef dstRef, const ENScnGpuFramebuffDstType type);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuFramebuff_h */
