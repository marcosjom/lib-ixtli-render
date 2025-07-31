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
    ENScnGpuFramebuffDstType_Renderbuff,
    ENScnGpuFramebuffDstType_OSView,
    //Count
    ENScnGpuFramebuffDstType_Count
} ENScnGpuFramebuffDstType;

//STScnGpuFramebuffChanges

typedef struct STScnGpuFramebuffChanges_ {
    ScnBOOL    bind;
} STScnGpuFramebuffChanges;

//STScnGpuFramebuffApiItf

typedef struct STScnGpuFramebuffApiItf_ {
    void        (*free)(void* data);
    //
    STScnSize2DU  (*getSize)(void* data, STScnRectU* dstViewport);
    ScnBOOL     (*syncSizeAndViewport)(void* data, const STScnSize2DU size, const STScnRectU viewport);
} STScnGpuFramebuffApiItf;


//ScnGpuFramebuffRef

SCN_REF_STRUCT_METHODS_DEC(ScnGpuFramebuff)

//

ScnBOOL     ScnGpuFramebuff_prepare(ScnGpuFramebuffRef ref, const STScnGpuFramebuffApiItf* itf, void* itfParam);

STScnSize2DU  ScnGpuFramebuff_getSize(ScnGpuFramebuffRef ref, STScnRectU* dstViewport);
ScnBOOL     ScnGpuFramebuff_syncSizeAndViewport(ScnGpuFramebuffRef ref, const STScnSize2DU size, const STScnRectU viewport);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuFramebuff_h */
