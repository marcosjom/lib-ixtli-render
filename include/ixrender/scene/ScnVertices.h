//
//  ScnVertices.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnVertices_h
#define ScnVertices_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/type/ScnPoint.h"
#include "ixrender/type/ScnColor.h"

#ifdef __cplusplus
extern "C" {
#endif

//ENScnVertexType

typedef enum ENScnVertexType {
    ENScnVertexType_2DColor = 0,    //2D, no texture
    ENScnVertexType_2DTex,          //2D, one texture
    ENScnVertexType_2DTex2,         //2D, two textures
    ENScnVertexType_2DTex3,         //2D, three textures
    //
    ENScnVertexType_Count
} ENScnVertexType;

//-------------------
//-- STScnVertexIdx
//-------------------

//These MACROs are useful for shader code.
#define ScnVertexIdx_IDX_idx    0u
#define ScnVertexIdx_SZ         4u

#define STScnVertexIdx_Zero     { 0 }

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

//Vertex with 3 textures
typedef struct STScnVertex2DTex3 {
    ScnFLOAT        x;
    ScnFLOAT        y;
    STScnColor8     color;
    STScnPoint2D    tex;
    STScnPoint2D    tex2;
    STScnPoint2D    tex3;
} STScnVertex2DTex3;


//

//-------------------
//-- STScnVertex2DF
//-------------------

//These MACROs are useful for shader code.
#define ScnVertex2DF_IDX_x        0u
#define ScnVertex2DF_IDX_y        4u
#define ScnVertex2DF_IDX_color_r  8u
#define ScnVertex2DF_IDX_color_g  12u
#define ScnVertex2DF_IDX_color_b  16u
#define ScnVertex2DF_IDX_color_a  20u
#define ScnVertex2DF_SZ           24u

//Vertex without texture
typedef struct STScnVertex2DF {
    ScnFLOAT        x;
    ScnFLOAT        y;
    STScnColor      color;
} STScnVertex2DF;

//--------------------
//-- STScnVertex2DTexF
//--------------------
//
//These MACROs are useful for shader code.
#define ScnVertex2DTexF_IDX_x         0u
#define ScnVertex2DTexF_IDX_y         4u
#define ScnVertex2DTexF_IDX_color_r   8u
#define ScnVertex2DTexF_IDX_color_g   12u
#define ScnVertex2DTexF_IDX_color_b   16u
#define ScnVertex2DTexF_IDX_color_a   20u
#define ScnVertex2DTexF_IDX_tex_x     24u
#define ScnVertex2DTexF_IDX_tex_y     28u
#define ScnVertex2DTexF_SZ            32u

//Vertex with one texture
typedef struct STScnVertex2DTexF {
    ScnFLOAT        x;
    ScnFLOAT        y;
    STScnColor      color;
    STScnPoint2D  tex;
} STScnVertex2DTexF;


//--------------------
//-- STScnVertex2DTex2F
//--------------------

//These MACROs are useful for shader code.
#define ScnVertex2DTex2F_IDX_x        0u
#define ScnVertex2DTex2F_IDX_y        4u
#define ScnVertex2DTex2F_IDX_color_r  8u
#define ScnVertex2DTex2F_IDX_color_g  12u
#define ScnVertex2DTex2F_IDX_color_b  16u
#define ScnVertex2DTex2F_IDX_color_a  20u
#define ScnVertex2DTex2F_IDX_tex_x    24u
#define ScnVertex2DTex2F_IDX_tex_y    28u
#define ScnVertex2DTex2F_IDX_tex2_x   32u
#define ScnVertex2DTex2F_IDX_tex2_y   36u
#define ScnVertex2DTex2F_SZ           40u

//Vertex with 2 textures
typedef struct STScnVertex2DTex2F {
    ScnFLOAT        x;
    ScnFLOAT        y;
    STScnColor      color;
    STScnPoint2D    tex;
    STScnPoint2D    tex2;
} STScnVertex2DTex2F;


//--------------------
//-- STScnVertex2DTex3F
//--------------------

//These MACROs are useful for shader code.
#define ScnVertex2DTex3F_IDX_x        0u
#define ScnVertex2DTex3F_IDX_y        4u
#define ScnVertex2DTex3F_IDX_color_r  8u
#define ScnVertex2DTex3F_IDX_color_g  12u
#define ScnVertex2DTex3F_IDX_color_b  16u
#define ScnVertex2DTex3F_IDX_color_a  20u
#define ScnVertex2DTex3F_IDX_tex_x    24u
#define ScnVertex2DTex3F_IDX_tex_y    28u
#define ScnVertex2DTex3F_IDX_tex2_x   32u
#define ScnVertex2DTex3F_IDX_tex2_y   36u
#define ScnVertex2DTex3F_IDX_tex3_x   40u
#define ScnVertex2DTex3F_IDX_tex3_y   44u
#define ScnVertex2DTex3F_SZ           48u

//Vertex with 3 textures
typedef struct STScnVertex2DTex3F {
    ScnFLOAT        x;
    ScnFLOAT        y;
    STScnColor      color;
    STScnPoint2D    tex;
    STScnPoint2D    tex2;
    STScnPoint2D    tex3;
} STScnVertex2DTex3F;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnVertices_h */
