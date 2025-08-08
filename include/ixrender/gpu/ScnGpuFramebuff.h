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

//ENScnGpuFramebuffDstType

typedef enum ENScnGpuFramebuffDstType {
    ENScnGpuFramebuffDstType_None = 0,
    ENScnGpuFramebuffDstType_Texture,
    ENScnGpuFramebuffDstType_Renderbuff,
    ENScnGpuFramebuffDstType_OSView,
    //Count
    ENScnGpuFramebuffDstType_Count
} ENScnGpuFramebuffDstType;

//STScnGpuFramebuffApiItf

typedef struct STScnGpuFramebuffApiItf {
    void            (*free)(void* data);
    //
    STScnSize2DU    (*getSize)(void* data);
    ScnBOOL         (*syncSize)(void* data, const STScnSize2DU size);
    //
    STScnGpuFramebuffProps (*getProps)(void* data);
    ScnBOOL         (*setProps)(void* data, const STScnGpuFramebuffProps* const props);
} STScnGpuFramebuffApiItf;


//ScnGpuFramebuffRef

#define ScnGpuFramebuffRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnGpuFramebuff)

//

ScnBOOL         ScnGpuFramebuff_prepare(ScnGpuFramebuffRef ref, const STScnGpuFramebuffApiItf* itf, void* itfParam);
void*           ScnGpuFramebuff_getApiItfParam(ScnGpuFramebuffRef ref);

STScnSize2DU    ScnGpuFramebuff_getSize(ScnGpuFramebuffRef ref);
ScnBOOL         ScnGpuFramebuff_syncSize(ScnGpuFramebuffRef ref, const STScnSize2DU size);
//
STScnGpuFramebuffProps  ScnGpuFramebuff_getProps(ScnGpuFramebuffRef ref);
ScnBOOL         ScnGpuFramebuff_setProps(ScnGpuFramebuffRef ref, const STScnGpuFramebuffProps* const props);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuFramebuff_h */
