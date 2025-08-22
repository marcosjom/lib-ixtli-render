//
//  ScnRenderShape.h
//  ixtli-render
//
//  Created by Marcos Ortega on 21/8/25.
//

#ifndef ScnRenderShape_h
#define ScnRenderShape_h

//ENScnRenderShape

/** @enum ENScnRenderShape
 *  @brief Render's drawing shapes.
 */

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

#endif /* ScnRenderShape_h */
