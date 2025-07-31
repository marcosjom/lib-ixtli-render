//
//  ScnModel2D.h
//  ixtli-render
//
//  Created by Marcos Ortega on 28/7/25.
//

#ifndef ScnModel2D_h
#define ScnModel2D_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/type/ScnColor.h"
#include "ixrender/type/ScnPoint.h"
#include "ixrender/type/ScnSize.h"
#include "ixrender/gpu/ScnGpuTexture.h"
#include "ixrender/scene/ScnTransform2D.h"
#include "ixrender/scene/ScnModelProps2D.h"
#include "ixrender/scene/ScnRenderCmd.h"
#include "ixrender/scene/ScnVertexbuffs.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnModel2DRef

SCN_REF_STRUCT_METHODS_DEC(ScnModel2D)

//

ScnBOOL             ScnModel2D_setVertexBuffs(ScnModel2DRef ref, ScnVertexbuffsRef vbuffs);

//props

STScnModelProps2D   ScnModel2D_getProps(ScnModel2DRef ref);

//color

STScnColor8         ScnModel2D_getColor8(ScnModel2DRef ref);
void                ScnModel2D_setColor8(ScnModel2DRef ref, const STScnColor8 color);
void                ScnModel2D_setColorRGBA8(ScnModel2DRef ref, const ScnUI8 r, const ScnUI8 g, const ScnUI8 b, const ScnUI8 a);

//transform

STScnTransform2D    ScnModel2D_getTransform(ScnModel2DRef ref);
STScnPoint2D        ScnModel2D_getTranslate(ScnModel2DRef ref);
STScnSize2D         ScnModel2D_getScale(ScnModel2DRef ref);
ScnFLOAT            ScnModel2D_getRotDeg(ScnModel2DRef ref);
ScnFLOAT            ScnModel2D_getRotRad(ScnModel2DRef ref);
void                ScnModel2D_setTranslate(ScnModel2DRef ref, const STScnPoint2D pos);
void                ScnModel2D_setTranslateXY(ScnModel2DRef ref, const ScnFLOAT x, const ScnFLOAT y);
void                ScnModel2D_setScale(ScnModel2DRef ref, const STScnSize2D s);
void                ScnModel2D_setScaleWH(ScnModel2DRef ref, const ScnFLOAT sw, const ScnFLOAT sh);
void                ScnModel2D_setRotDeg(ScnModel2DRef ref, const ScnFLOAT deg);
void                ScnModel2D_setRotRad(ScnModel2DRef ref, const ScnFLOAT rad);

//draw commands

void                ScnModel2D_resetDrawCmds(ScnModel2DRef ref);
//
STScnVertex2DPtr    ScnModel2D_addDraw(ScnModel2DRef ref, const ENScnRenderShape shape, const ScnUI32 count);
STScnVertex2DTexPtr ScnModel2D_addDrawTex(ScnModel2DRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnGpuTextureRef t0);
STScnVertex2DTex2Ptr ScnModel2D_addDrawTex2(ScnModel2DRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnGpuTextureRef t0, ScnGpuTextureRef t1);
STScnVertex2DTex3Ptr ScnModel2D_addDrawTex3(ScnModel2DRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnGpuTextureRef t0, ScnGpuTextureRef t1, ScnGpuTextureRef t2);
//
STScnVertexIdxPtr   ScnModel2D_addDrawIndexed(ScnModel2DRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, const ScnUI32 countVerts, STScnVertex2DPtr* dstVerts);
STScnVertexIdxPtr   ScnModel2D_addDrawIndexedTex(ScnModel2DRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnGpuTextureRef t0, const ScnUI32 countVerts, STScnVertex2DTexPtr* dstVerts);
STScnVertexIdxPtr   ScnModel2D_addDrawIndexedTex2(ScnModel2DRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnGpuTextureRef t0, ScnGpuTextureRef t1, const ScnUI32 countVerts, STScnVertex2DTex2Ptr* dstVerts);
STScnVertexIdxPtr   ScnModel2D_addDrawIndexedTex3(ScnModel2DRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnGpuTextureRef t0, ScnGpuTextureRef t1, ScnGpuTextureRef t2, const ScnUI32 countVerts, STScnVertex2DTex3Ptr* dstVerts);

//draw commands to consumer

ScnBOOL             ScnModel2D_sendRenderCmds(ScnModel2DRef ref, STScnModel2DPushItf* itf, void* itfParam);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnModel2D_h */
