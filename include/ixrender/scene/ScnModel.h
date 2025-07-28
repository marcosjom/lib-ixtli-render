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
#include "ixrender/ScnRender.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnModelRef

SCN_REF_STRUCT_METHODS_DEC(ScnModel)

//props

STScnModelProps ScnModel_getProps(STScnModelRef ref);

//color

STScnColor8     ScnModel_getColor8(STScnModelRef ref);
void            ScnModel_setColor8(STScnModelRef ref, const STScnColor8 color);
void            ScnModel_setColorRGBA8(STScnModelRef ref, const ScnUI8 r, const ScnUI8 g, const ScnUI8 b, const ScnUI8 a);

//transform

STScnTransform  ScnModel_getTransform(STScnModelRef ref);
STScnPoint      ScnModel_getTranslate(STScnModelRef ref);
STScnSize       ScnModel_getScale(STScnModelRef ref);
ScnFLOAT        ScnModel_getRotDeg(STScnModelRef ref);
ScnFLOAT        ScnModel_getRotRad(STScnModelRef ref);
void            ScnModel_setTranslate(STScnModelRef ref, const STScnPoint pos);
void            ScnModel_setTranslateXY(STScnModelRef ref, const ScnFLOAT x, const ScnFLOAT y);
void            ScnModel_setScale(STScnModelRef ref, const STScnSize s);
void            ScnModel_setScaleWH(STScnModelRef ref, const ScnFLOAT sw, const ScnFLOAT sh);
void            ScnModel_setRotDeg(STScnModelRef ref, const ScnFLOAT deg);
void            ScnModel_setRotRad(STScnModelRef ref, const ScnFLOAT rad);

//draw commands

void            ScnModel_resetDrawCmds(STScnModelRef ref);
//
ScnBOOL         ScnModel_addDrawCmd(STScnModelRef ref, const ENScnRenderShape shape, STScnVertexPtr verts, const ScnUI32 iFirst, const ScnUI32 count, STScnRenderRef rndr);
ScnBOOL         ScnModel_addDrawTex(STScnModelRef ref, const ENScnRenderShape shape, STScnVertexTexPtr verts, const ScnUI32 iFirst, const ScnUI32 count, STScnGpuTextureRef t0, STScnRenderRef rndr);
ScnBOOL         ScnModel_addDrawTex2(STScnModelRef ref, const ENScnRenderShape shape, STScnVertexTex2Ptr verts, const ScnUI32 iFirst, const ScnUI32 count, STScnGpuTextureRef t0, STScnGpuTextureRef t1, STScnRenderRef rndr);
ScnBOOL         ScnModel_addDrawTex3(STScnModelRef ref, const ENScnRenderShape shape, STScnVertexTex3Ptr verts, const ScnUI32 iFirst, const ScnUI32 count, STScnGpuTextureRef t0, STScnGpuTextureRef t1, STScnGpuTextureRef t2, STScnRenderRef rndr);
//
ScnBOOL         ScnModel_addDrawIndexedCmd(STScnModelRef ref, const ENScnRenderShape shape, STScnVertexIdxPtr idxs, const ScnUI32 iFirst, const ScnUI32 count, STScnRenderRef rndr);
ScnBOOL         ScnModel_addDrawIndexedTex(STScnModelRef ref, const ENScnRenderShape shape, STScnVertexIdxPtr idxs, const ScnUI32 iFirst, const ScnUI32 count, STScnGpuTextureRef t0, STScnRenderRef rndr);
ScnBOOL         ScnModel_addDrawIndexedTex2(STScnModelRef ref, const ENScnRenderShape shape, STScnVertexIdxPtr idxs, const ScnUI32 iFirst, const ScnUI32 count, STScnGpuTextureRef t0, STScnGpuTextureRef t1, STScnRenderRef rndr);
ScnBOOL         ScnModel_addDrawIndexedTex3(STScnModelRef ref, const ENScnRenderShape shape, STScnVertexIdxPtr idxs, const ScnUI32 iFirst, const ScnUI32 count, STScnGpuTextureRef t0, STScnGpuTextureRef t1, STScnGpuTextureRef t2, STScnRenderRef rndr);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnModel_h */
