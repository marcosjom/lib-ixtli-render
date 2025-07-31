//
//  ScnRenderCmd.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnRenderCmd_h
#define ScnRenderCmd_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/scene/ScnVertices.h"
#include "ixrender/scene/ScnVertexbuffs.h"
#include "ixrender/scene/ScnModelProps2D.h"
#include "ixrender/scene/ScnTexture.h"

#ifdef __cplusplus
extern "C" {
#endif

//ENScnRenderShape

typedef enum ENScnRenderShape_ {
    ENScnRenderShape_Compute = 0, //compute vertices but not drawn
    //
    ENScnRenderShape_Texture,     //same as 'ENScnRenderShape_TriangStrip' with possible bitblit-optimization if matrix has no rotation.
    ENScnRenderShape_TriangStrip, //triangles-strip, most common shape
    ENScnRenderShape_TriangFan,   //triangles-fan
    //
    ENScnRenderShape_LineStrip,   //lines-strip
    ENScnRenderShape_LineLoop,    //lines-loop
    ENScnRenderShape_Lines,       //lines
    ENScnRenderShape_Points,      //points
    //Count
    ENScnRenderShape_Count
} ENScnRenderShape;

//ENScnRenderCmd

typedef enum ENScnRenderCmd_ {
    ENScnRenderCmd_None = 0       //do nothing
    //models
    , ENScnRenderCmd_SetTransformOffset //sets the positions of the 'STScnGpuModelProps2D' to be applied for the drawing cmds
    , ENScnRenderCmd_SetVertexBuff  //activates the vertex buffer
    //modes
    , ENScnRenderCmd_MaskModePush   //pushes drawing-mask mode, where only the alpha value is affected
    , ENScnRenderCmd_MaskModePop    //pop
    //drawing
    , ENScnRenderCmd_SetTexture     //activates the texture in a specific slot-index
    , ENScnRenderCmd_SetVertsType   //activates the type of vertices to be used (indexes and vertices)
    , ENScnRenderCmd_DrawVerts      //draws something using the vertices
    , ENScnRenderCmd_DrawIndexes    //draws something using the vertices indexes
    //Count
    , ENScnRenderCmd_Count
} ENScnRenderCmd;

//ENScnRenderCmd

typedef struct STScnRenderCmd_ {
    ENScnRenderCmd cmdId;          //id of the command
    union {
        //ENScnRenderCmd_SetTransformOffset
        struct {
            ScnUI32         offset;
        } setTransformOffset;
        //ENScnRenderCmd_SetVertexBuff
        struct {
            ScnVertexbuffRef ref;
        } setVertexBuff;
        //ENScnRenderCmd_MaskModePush
        //  nothing
        //ENScnRenderCmd_MaskModePop
        //  nothing
        //ENScnRenderCmd_SetTexture
        struct {
            ScnUI32         index;   //slot-index
            ScnTextureRef   tex;     //ScnGpuTextureRef  tex;  //texture-id
        } setTexture;
        //ENScnRenderCmd_SetVertsType
        struct {
            ENScnVertexType type;
            
        } setVertsType;
        //ENScnRenderCmd_DrawVerts
        struct {
            ENScnRenderShape shape;
            ScnUI32    iFirst;
            ScnUI32    count;
        } drawVerts;
        //ENScnRenderCmd_DrawIndexes
        struct {
            ENScnRenderShape shape;
            ScnUI32    iFirst;
            ScnUI32    count;
        } drawIndexes;
    };
} STScnRenderCmd;

//ENScnModelDrawCmdType

typedef enum ENScnModelDrawCmdType_ {
    ENScnModelDrawCmdType_Undef = 0,
    //2D vertex drawing
    ENScnModelDrawCmdType_2Dv0,
    ENScnModelDrawCmdType_2Dv1,
    ENScnModelDrawCmdType_2Dv2,
    ENScnModelDrawCmdType_2Dv3,
    //2D indexed drawing
    ENScnModelDrawCmdType_2Di0,
    ENScnModelDrawCmdType_2Di1,
    ENScnModelDrawCmdType_2Di2,
    ENScnModelDrawCmdType_2Di3,
    //
    ENScnModelDrawCmdType_Count,
} ENScnModelDrawCmdType;

//STScnModel2DCmd

typedef struct STScnModel2DCmd_ {
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
    struct {
        ScnGpuTextureRef    t0;
        ScnGpuTextureRef    t1;
        ScnGpuTextureRef    t2;
    } texs;
} STScnModel2DCmd;

void ScnModel2DCmd_init(STScnModel2DCmd* obj);
void ScnModel2DCmd_destroy(STScnModel2DCmd* obj);

//STScnGpuBufferApiItf

#define STScnModel2DPushItf_Zero  { NULL }

typedef struct STScnModel2DPushItf_ {
    ScnBOOL (*addCommandsWithProps)(void* data, const STScnModelProps2D* const props, const STScnModel2DCmd* const cmds, const ScnUI32 cmdsSz);
} STScnModel2DPushItf;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRenderCmd_h */
