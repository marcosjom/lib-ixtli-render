//
//  ScnVertex.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnVertex_h
#define ScnVertex_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/type/ScnPoint.h"
#include "ixrender/type/ScnColor.h"
#include "ixrender/scene/ScnVertexType.h"

#ifdef __cplusplus
extern "C" {
#endif

//-------------------
//-- STScnVertexIdx
//-------------------

//These MACROs are useful for shader code.
#define ScnVertexIdx_IDX_idx    0u
#define ScnVertexIdx_SZ         4u

#define STScnVertexIdx_Zero     { 0 }

/** @struct STScnVertexIdx
 *  @brief Vertex index.
 *  @var STScnVertexIdx::i
 *  Index's value.
 */

//Vertex without texture
typedef struct STScnVertexIdx {
    ScnUI32     i;
} STScnVertexIdx;

//-------------------
//-- STScnVertex2D
//-------------------

//These MACROs are useful for shader code.
#define ScnVertex2D_IDX_x         0u
#define ScnVertex2D_IDX_y         4u
#define ScnVertex2D_IDX_color     8u
#define ScnVertex2D_SZ            12u

#define STScnVertex2D_Zero     { 0.f, 0.f, STScnColor8_Zero }

/** @struct STScnVertex2D
 *  @brief Zero texture vertex.
 *  @var STScnVertex2D::x
 *  X position.
 *  @var STScnVertex2D::y
 *  Y position.
 *  @var STScnVertex2D::color
 *  Color value.
 */

//Vertex without texture
typedef struct STScnVertex2D {
    ScnFLOAT        x;
    ScnFLOAT        y;
    STScnColor8     color;
} STScnVertex2D;

//--------------------
//-- STScnVertex2DTex
//--------------------

//These MACROs are useful for shader code.
#define ScnVertex2DTex_IDX_x      0u
#define ScnVertex2DTex_IDX_y      4u
#define ScnVertex2DTex_IDX_color  8u
#define ScnVertex2DTex_IDX_tex_x  12u
#define ScnVertex2DTex_IDX_tex_y  16u
#define ScnVertex2DTex_SZ         20u

#define STScnVertex2DTex_Zero     { 0.f, 0.f, STScnColor8_Zero, STScnPoint2D_Zero }

/** @struct STScnVertex2DTex
 *  @brief One texture vertex.
 *  @var STScnVertex2DTex::x
 *  X position.
 *  @var STScnVertex2DTex::y
 *  Y position.
 *  @var STScnVertex2DTex::color
 *  Color value.
 *  @var STScnVertex2DTex::tex
 *  Texture's coordinate.
 */

//Vertex with one texture
typedef struct STScnVertex2DTex {
    ScnFLOAT        x;
    ScnFLOAT        y;
    STScnColor8     color;
    STScnPoint2D    tex;
} STScnVertex2DTex;


//--------------------
//-- STScnVertex2DTex2
//--------------------

//These MACROs are useful for shader code.
#define ScnVertex2DTex2_IDX_x       0u
#define ScnVertex2DTex2_IDX_y       4u
#define ScnVertex2DTex2_IDX_color   8u
#define ScnVertex2DTex2_IDX_tex_x   12u
#define ScnVertex2DTex2_IDX_tex_y   16u
#define ScnVertex2DTex2_IDX_tex2_x  20u
#define ScnVertex2DTex2_IDX_tex2_y  24u
#define ScnVertex2DTex2_SZ          28u

#define STScnVertex2DTex2_Zero     { 0.f, 0.f, STScnColor8_Zero, STScnPoint2D_Zero, STScnPoint2D_Zero }

/** @struct STScnVertex2DTex2
 *  @brief Two textures vertex.
 *  @var STScnVertex2DTex2::x
 *  X position.
 *  @var STScnVertex2DTex2::y
 *  Y position.
 *  @var STScnVertex2DTex2::color
 *  Color value.
 *  @var STScnVertex2DTex2::tex
 *  First texture's coordinate.
 *  @var STScnVertex2DTex2::tex2
 *  Second texture's coordinate.
 */

//Vertex with 2 textures
typedef struct STScnVertex2DTex2 {
    ScnFLOAT        x;
    ScnFLOAT        y;
    STScnColor8     color;
    STScnPoint2D    tex;
    STScnPoint2D    tex2;
} STScnVertex2DTex2;

//--------------------
//-- STScnVertex2DTex3
//--------------------

//These MACROs are useful for shader code.
#define ScnVertex2DTex3_IDX_x         0u
#define ScnVertex2DTex3_IDX_y         4u
#define ScnVertex2DTex3_IDX_color     8u
#define ScnVertex2DTex3_IDX_tex_x     12u
#define ScnVertex2DTex3_IDX_tex_y     16u
#define ScnVertex2DTex3_IDX_tex2_x    20u
#define ScnVertex2DTex3_IDX_tex2_y    24u
#define ScnVertex2DTex3_IDX_tex3_x    28u
#define ScnVertex2DTex3_IDX_tex3_y    32u
#define ScnVertex2DTex3_SZ            36u

#define STScnVertex2DTex3_Zero     { 0.f, 0.f, STScnColor8_Zero, STScnPoint2D_Zero, STScnPoint2D_Zero, STScnPoint2D_Zero }

/** @struct STScnVertex2DTex3
 *  @brief Three textures vertex.
 *  @var STScnVertex2DTex3::x
 *  X position.
 *  @var STScnVertex2DTex3::y
 *  Y position.
 *  @var STScnVertex2DTex3::color
 *  Color value.
 *  @var STScnVertex2DTex3::tex
 *  First texture's coordinate.
 *  @var STScnVertex2DTex3::tex2
 *  Second texture's coordinate.
 *  @var STScnVertex2DTex3::tex3
 *  Third texture's coordinate.
 */

//Vertex with 3 textures
typedef struct STScnVertex2DTex3 {
    ScnFLOAT        x;
    ScnFLOAT        y;
    STScnColor8     color;
    STScnPoint2D    tex;
    STScnPoint2D    tex2;
    STScnPoint2D    tex3;
} STScnVertex2DTex3;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnVertices_h */
