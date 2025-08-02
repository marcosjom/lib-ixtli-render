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
#include "ixrender/scene/ScnFramebuff.h"
#include "ixrender/scene/ScnNode2dProps.h"
#include "ixrender/scene/ScnTexture.h"

#ifdef __cplusplus
extern "C" {
#endif

//ENScnRenderShape

typedef enum ENScnRenderShape {
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

typedef enum ENScnRenderCmd {
    ENScnRenderCmd_None = 0       //do nothing
    //framebuffers
    , ENScnRenderCmd_ActivateFramebuff
    , ENScnRenderCmd_SetFramebuffProps
    //models
    , ENScnRenderCmd_SetTransformOffset //sets the positions of the 'STScnGpuModelProps2D' to be applied for the drawing cmds
    , ENScnRenderCmd_SetVertexBuff  //activates the vertex buffer
    , ENScnRenderCmd_SetTexture     //activates the texture in a specific slot-index
    //modes
    , ENScnRenderCmd_MaskModePush   //pushes drawing-mask mode, where only the alpha value is affected
    , ENScnRenderCmd_MaskModePop    //pop
    //drawing
    , ENScnRenderCmd_DrawVerts      //draws something using the vertices
    , ENScnRenderCmd_DrawIndexes    //draws something using the vertices indexes
    //Count
    , ENScnRenderCmd_Count
} ENScnRenderCmd;

//ENScnRenderCmd

typedef struct STScnRenderCmd {
    ENScnRenderCmd cmdId;          //id of the command
    union {
        //ENScnRenderCmd_ActivateFramebuff
        struct {
            ScnFramebuffRef ref;
            ScnUI32         offset; //position of data in viewPropsBuff
            STScnRectU      viewport;
        } activateFramebuff;
        //ENScnRenderCmd_SetFramebuffProps
        struct {
            ScnUI32         offset; //position of data in viewPropsBuff
            STScnRectU      viewport;
        } setFramebuffProps;
        //ENScnRenderCmd_SetTransformOffset
        struct {
            ScnUI32         offset; //position of data in nodesPropsBuff
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

typedef enum ENScnModelDrawCmdType {
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

//STScnModel2dCmd

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
    struct {
        ScnGpuTextureRef    t0;
        ScnGpuTextureRef    t1;
        ScnGpuTextureRef    t2;
    } texs;
} STScnModel2dCmd;

void ScnModel2dCmd_init(STScnModel2dCmd* obj);
void ScnModel2dCmd_destroy(STScnModel2dCmd* obj);

//STScnGpuBufferApiItf

#define STScnModel2dPushItf_Zero  { NULL }

typedef struct STScnModel2dPushItf {
    ScnBOOL (*addCommandsWithProps)(void* data, const STScnGpuModelProps2D* const props, const STScnModel2dCmd* const cmds, const ScnUI32 cmdsSz);
} STScnModel2dPushItf;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRenderCmd_h */
