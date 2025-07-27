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
    ENScnRenderCmd_None = 0,       //do nothing
    //modes
    ENScnRenderCmd_MaskModePush,   //pushes drawing-mask mode, where only the alpha value is affected
    ENScnRenderCmd_MaskModePop,    //pop
    //drawing
    ENScnRenderCmd_SetTexture,     //activates the texture in a specific slot-index
    ENScnRenderCmd_SetVertsType,   //activates the type of vertices to be used (indexes and vertices)
    ENScnRenderCmd_DrawVerts,      //draws something using the vertices
    ENScnRenderCmd_DrawIndexes,    //draws something using the vertices indexes
    //Count
    ENScnRenderCmd_Count
} ENScnRenderCmd;

//ENScnRenderCmd

typedef struct STScnRenderCmd_ {
    ENScnRenderCmd cmdId;          //id of the command
    union {
        //ENScnRenderCmd_MaskModePush
        //  nothing
        //ENScnRenderCmd_MaskModePop
        //  nothing
        //ENScnRenderCmd_SetTexture
        struct {
            ScnUI32    index;  //slot-index
            ScnUI32    tex; //STScnGpuTextureRef  tex;  //texture-id
        } setTexture;
        //ENScnRenderCmd_SetVertsType
        struct {
            ENScnVertexType type;
        } setVertsType;
        //ENScnRenderCmd_DrawVerts
        struct {
            ENScnRenderShape mode;
            ScnUI32    iFirst;
            ScnUI32    count;
        } drawVerts;
        //ENScnRenderCmd_DrawIndexes
        struct {
            ENScnRenderShape mode;
            ScnUI32    iFirst;
            ScnUI32    count;
        } drawIndexes;
    };
} STScnRenderCmd;


#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRenderCmd_h */
