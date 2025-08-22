//
//  ScnRenderCmd.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnRenderCmd_h
#define ScnRenderCmd_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/scene/ScnVertex.h"
#include "ixrender/scene/ScnVertexbuffs.h"
#include "ixrender/scene/ScnFramebuff.h"
#include "ixrender/scene/ScnNode2dProps.h"
#include "ixrender/scene/ScnTexture.h"
#include "ixrender/scene/ScnRenderShape.h"

#ifdef __cplusplus
extern "C" {
#endif

//ENScnRenderCmd

/** @enum ENScnRenderCmd
 *  @brief Render's commands.
 */

typedef enum ENScnRenderCmd {
    ENScnRenderCmd_None = 0       //do nothing
    //framebuffers
    , ENScnRenderCmd_ActivateFramebuff
    , ENScnRenderCmd_SetFramebuffProps
    //models
    , ENScnRenderCmd_SetTransformOffset //sets the positions of the 'STScnGpuModelProps2d' to be applied for the drawing cmds
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

/** @struct STScnRenderCmd
 *  @brief Render packaged command.
 *  @var STScnRenderCmd::cmdId
 *  Command identifier.
 */

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
            ScnBOOL         isFirstUse; //is first time the ScnVertexbuffRef appears in this cmds queue
        } setVertexBuff;
        //ENScnRenderCmd_SetTexture
        struct {
            ScnTextureRef   ref;
            ScnUI32         index;   //slot-index
            ScnBOOL         isFirstUse; //is first time the ScnVertexbuffRef appears in this cmds queue
        } setTexture;
        //ENScnRenderCmd_MaskModePush
        //  nothing
        //ENScnRenderCmd_MaskModePop
        //  nothing
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

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRenderCmd_h */
