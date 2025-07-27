//
//  ScnGpuRenderbuff.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnGpuRenderbuff_h
#define ScnGpuRenderbuff_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/scene/ScnBitmap.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnGpuRenderbuffCfg

#define STScnGpuRenderbuffCfg_Zero   { ENScnBitmapColor_undef, 0, 0 }

typedef struct STScnGpuRenderbuffCfg_ {
    ENScnBitmapColor color;
    ScnUI32         width;
    ScnUI32         height;
} STScnGpuRenderbuffCfg;

//STScnGpuRenderbuffChanges

typedef struct STScnGpuRenderbuffChanges_ {
    ScnUI32 dummy;  //nothing
} STScnGpuRenderbuffChanges;

//STScnGpuRenderbuffApiItf

typedef struct STScnGpuRenderbuffApiItf_ {
    void* (*create)(const STScnGpuRenderbuffCfg* cfg, void* usrData);
    void  (*destroy)(void* data, void* usrData);
    //
    ScnBOOL  (*sync)(void* data, const STScnGpuRenderbuffCfg* cfg, const STScnGpuRenderbuffChanges* changes, void* usrData);
} STScnGpuRenderbuffApiItf;

//STScnGpuRenderbuffRef

SCN_REF_STRUCT_METHODS_DEC(ScnGpuRenderbuff)

//

ScnBOOL ScnGpuRenderbuff_prepare(STScnGpuRenderbuffRef ref, const STScnGpuRenderbuffCfg* cfg, const STScnGpuRenderbuffApiItf* itf, void* itfParam);


#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuRenderbuff_h */
