//
//  ScnModel.h
//  ixtli-render
//
//  Created by Marcos Ortega on 28/7/25.
//

#ifndef ScnModel_h
#define ScnModel_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/type/ScnColor.h"
#include "ixrender/type/ScnPoint.h"
#include "ixrender/type/ScnSize.h"
#include "ixrender/gpu/ScnGpuTexture.h"
#include "ixrender/scene/ScnTransform.h"
#include "ixrender/scene/ScnModelProps.h"
#include "ixrender/scene/ScnRenderCmd.h"
#include "ixrender/scene/ScnVertexbuffs.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnModelRef

SCN_REF_STRUCT_METHODS_DEC(ScnModel)

//

ScnBOOL         ScnModel_setVertexBuffs(ScnModelRef ref, ScnVertexbuffsRef vbuffs);

//props

STScnModelProps ScnModel_getProps(ScnModelRef ref);

//color

STScnColor8     ScnModel_getColor8(ScnModelRef ref);
void            ScnModel_setColor8(ScnModelRef ref, const STScnColor8 color);
void            ScnModel_setColorRGBA8(ScnModelRef ref, const ScnUI8 r, const ScnUI8 g, const ScnUI8 b, const ScnUI8 a);

//transform

STScnTransform  ScnModel_getTransform(ScnModelRef ref);
STScnPoint      ScnModel_getTranslate(ScnModelRef ref);
STScnSize       ScnModel_getScale(ScnModelRef ref);
ScnFLOAT        ScnModel_getRotDeg(ScnModelRef ref);
ScnFLOAT        ScnModel_getRotRad(ScnModelRef ref);
void            ScnModel_setTranslate(ScnModelRef ref, const STScnPoint pos);
void            ScnModel_setTranslateXY(ScnModelRef ref, const ScnFLOAT x, const ScnFLOAT y);
void            ScnModel_setScale(ScnModelRef ref, const STScnSize s);
void            ScnModel_setScaleWH(ScnModelRef ref, const ScnFLOAT sw, const ScnFLOAT sh);
void            ScnModel_setRotDeg(ScnModelRef ref, const ScnFLOAT deg);
void            ScnModel_setRotRad(ScnModelRef ref, const ScnFLOAT rad);

//draw commands

void                ScnModel_resetDrawCmds(ScnModelRef ref);
//
STScnVertexPtr      ScnModel_addDraw(ScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 count);
STScnVertexTexPtr   ScnModel_addDrawTex(ScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnGpuTextureRef t0);
STScnVertexTex2Ptr  ScnModel_addDrawTex2(ScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnGpuTextureRef t0, ScnGpuTextureRef t1);
STScnVertexTex3Ptr  ScnModel_addDrawTex3(ScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnGpuTextureRef t0, ScnGpuTextureRef t1, ScnGpuTextureRef t2);
//
STScnVertexIdxPtr   ScnModel_addDrawIndexed(ScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, const ScnUI32 countVerts, STScnVertexPtr* dstVerts);
STScnVertexIdxPtr   ScnModel_addDrawIndexedTex(ScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnGpuTextureRef t0, const ScnUI32 countVerts, STScnVertexTexPtr* dstVerts);
STScnVertexIdxPtr   ScnModel_addDrawIndexedTex2(ScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnGpuTextureRef t0, ScnGpuTextureRef t1, const ScnUI32 countVerts, STScnVertexTex2Ptr* dstVerts);
STScnVertexIdxPtr   ScnModel_addDrawIndexedTex3(ScnModelRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnGpuTextureRef t0, ScnGpuTextureRef t1, ScnGpuTextureRef t2, const ScnUI32 countVerts, STScnVertexTex3Ptr* dstVerts);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnModel_h */
