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
#include "ixrender/scene/ScnModelProps.h"
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
    , ENScnRenderCmd_SetTransformOffset //sets the positions of the 'STScnGpuTransform' to be applied for the drawing cmds
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
    //
    ENScnModelDrawCmdType_v0,
    ENScnModelDrawCmdType_v1,
    ENScnModelDrawCmdType_v2,
    ENScnModelDrawCmdType_v3,
    //
    ENScnModelDrawCmdType_i0,
    ENScnModelDrawCmdType_i1,
    ENScnModelDrawCmdType_i2,
    ENScnModelDrawCmdType_i3,
    //
    ENScnModelDrawCmdType_Count,
} ENScnModelDrawCmdType;

//STScnModelDrawCmd

typedef struct STScnModelDrawCmd_ {
    ENScnModelDrawCmdType   type;
    ENScnRenderShape        shape;
    ScnVertexbuffsRef       vbuffs;
    //verts
    struct {
        union {
            STScnVertexPtr      v0;
            STScnVertexTexPtr   v1;
            STScnVertexTex2Ptr  v2;
            STScnVertexTex3Ptr  v3;
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
} STScnModelDrawCmd;

void ScnModelDrawCmd_init(STScnModelDrawCmd* obj);
void ScnModelDrawCmd_destroy(STScnModelDrawCmd* obj);

//STScnGpuBufferApiItf

#define STScnModelPushItf_Zero  { NULL }

typedef struct STScnModelPushItf_ {
    ScnBOOL (*addCommandsWithProps)(void* data, const STScnModelProps* const props, const STScnModelDrawCmd* const cmds, const ScnUI32 cmdsSz);
} STScnModelPushItf;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRenderCmd_h */
