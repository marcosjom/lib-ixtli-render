//
//  ScnModel2d.h
//  ixtli-render
//
//  Created by Marcos Ortega on 28/7/25.
//

#ifndef ScnModel2d_h
#define ScnModel2d_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/type/ScnColor.h"
#include "ixrender/type/ScnPoint.h"
#include "ixrender/type/ScnSize.h"
#include "ixrender/gpu/ScnGpuTexture.h"
#include "ixrender/scene/ScnTransform2D.h"
#include "ixrender/scene/ScnNode2dProps.h"
#include "ixrender/scene/ScnRenderCmd.h"
#include "ixrender/scene/ScnVertexbuffs.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnModel2dRef

SCN_REF_STRUCT_METHODS_DEC(ScnModel2d)

//

ScnBOOL             ScnModel2d_setVertexBuffs(ScnModel2dRef ref, ScnVertexbuffsRef vbuffs);

//draw commands

//Removes all drawCmds and frees all vertex pointers.
void                ScnModel2d_resetDrawCmds(ScnModel2dRef ref);
//
//The Ptrs returned by these methods are valid until 'ScnModel2d_resetDrawCmds' is called or the ScnModel2d is freed.
STScnVertex2DPtr    ScnModel2d_addDraw(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 count);
STScnVertex2DTexPtr ScnModel2d_addDrawTex(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnGpuTextureRef t0);
STScnVertex2DTex2Ptr ScnModel2d_addDrawTex2(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnGpuTextureRef t0, ScnGpuTextureRef t1);
STScnVertex2DTex3Ptr ScnModel2d_addDrawTex3(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 count, ScnGpuTextureRef t0, ScnGpuTextureRef t1, ScnGpuTextureRef t2);
//Call these if you updated the vertices values after last render pass.
ScnBOOL             ScnModel2d_v0FlagForSync(ScnModel2dRef ref, STScnVertex2DPtr ptr, const ScnUI32 count);
ScnBOOL             ScnModel2d_v1FlagForSync(ScnModel2dRef ref, STScnVertex2DTexPtr ptr, const ScnUI32 count);
ScnBOOL             ScnModel2d_v2FlagForSync(ScnModel2dRef ref, STScnVertex2DTex2Ptr ptr, const ScnUI32 count);
ScnBOOL             ScnModel2d_v3FlagForSync(ScnModel2dRef ref, STScnVertex2DTex3Ptr ptr, const ScnUI32 count);
//The Ptrs returned by these methods are valid until 'ScnModel2d_resetDrawCmds' is called or the ScnModel2d is freed.
STScnVertexIdxPtr   ScnModel2d_addDrawIndexed(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, const ScnUI32 countVerts, STScnVertex2DPtr* dstVerts);
STScnVertexIdxPtr   ScnModel2d_addDrawIndexedTex(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnGpuTextureRef t0, const ScnUI32 countVerts, STScnVertex2DTexPtr* dstVerts);
STScnVertexIdxPtr   ScnModel2d_addDrawIndexedTex2(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnGpuTextureRef t0, ScnGpuTextureRef t1, const ScnUI32 countVerts, STScnVertex2DTex2Ptr* dstVerts);
STScnVertexIdxPtr   ScnModel2d_addDrawIndexedTex3(ScnModel2dRef ref, const ENScnRenderShape shape, const ScnUI32 countIdxs, ScnGpuTextureRef t0, ScnGpuTextureRef t1, ScnGpuTextureRef t2, const ScnUI32 countVerts, STScnVertex2DTex3Ptr* dstVerts);
//Call these if you updated the vertices values after last render pass.
ScnBOOL             ScnModel2d_i0FlagForSync(ScnModel2dRef ref, STScnVertexIdxPtr ptr, const ScnUI32 count);
ScnBOOL             ScnModel2d_i1FlagForSync(ScnModel2dRef ref, STScnVertexIdxPtr ptr, const ScnUI32 count);
ScnBOOL             ScnModel2d_i2FlagForSync(ScnModel2dRef ref, STScnVertexIdxPtr ptr, const ScnUI32 count);
ScnBOOL             ScnModel2d_i3FlagForSync(ScnModel2dRef ref, STScnVertexIdxPtr ptr, const ScnUI32 count);

//draw commands to consumer

ScnBOOL             ScnModel2d_sendRenderCmds(ScnModel2dRef ref, const STScnGpuModelProps2D* const props, STScnModel2dPushItf* itf, void* itfParam);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnModel2d_h */
