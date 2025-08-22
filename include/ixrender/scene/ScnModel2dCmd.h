//
//  ScnModel2dCmd.h
//  ixtli-render
//
//  Created by Marcos Ortega on 21/8/25.
//

#ifndef ScnModel2dCmd_h
#define ScnModel2dCmd_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/scene/ScnModelDrawCmdType.h"
#include "ixrender/scene/ScnRenderShape.h"
#include "ixrender/scene/ScnVertexbuffs.h"
#include "ixrender/scene/ScnVertexbuff.h"
#include "ixrender/scene/ScnVertex.h"
#include "ixrender/scene/ScnTexture.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnModel2dCmd

/** @struct STScnModel2dCmd
 *  @brief Draw command for a 2d model.
 *  @var STScnModel2dCmd::type
 *  Draw command type, by vertex or index type.
 *  @var STScnModel2dCmd::shape
 *  Draw shape
 *  @var STScnModel2dCmd::verts
 *  Vertices to draw union, the specific interval value depends of 'type' value.
 *  @var STScnModel2dCmd::idxs
 *  Indices to draw union, the specific interval value depends of 'type' value.
 *  @var STScnModel2dCmd::texs
 *  References to textures used, the ammount of textures depends of 'type'' value.
 */

typedef struct STScnModel2dCmd {
    ENScnModelDrawCmdType   type;
    ENScnRenderShape        shape;
    ScnVertexbuffsRef       vbuffs;
    //verts
    struct {
        union {
            STScnVertex2DPtr    v0;
            STScnVertex2DTexPtr v1;
            STScnVertex2DTex2Ptr v2;
            STScnVertex2DTex3Ptr v3;
        };
        ScnUI32 count;
    } verts;
    //idxs
    struct {
        union {
            STScnVertexIdxPtr   i0;
            STScnVertexIdxPtr   i1;
            STScnVertexIdxPtr   i2;
            STScnVertexIdxPtr   i3;
        };
        ScnUI32 count;
    } idxs;
    //texs
    ScnTextureRef texs[ENScnGpuTextureIdx_Count];
} STScnModel2dCmd;

/**
 * @brief Draw commnad initialization; zeroes the structure.
 * @param obj Reference to object.
 */
void ScnModel2dCmd_init(STScnModel2dCmd* obj);

/**
 * @brief Draw commnad destruction; releases retained textures.
 * @param obj Reference to object.
 */

void ScnModel2dCmd_destroy(STScnModel2dCmd* obj);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnModel2dCmd_h */
