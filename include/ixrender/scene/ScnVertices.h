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

typedef enum ENScnVertexType_ {
    ENScnVertexType_Color = 0,    //no texture
    ENScnVertexType_Tex,          //one texture
    ENScnVertexType_Tex2,         //two textures
    ENScnVertexType_Tex3,         //three textures
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
typedef struct STScnVertexIdx_ {
    ScnUI32     i;
} STScnVertexIdx;

//-------------------
//-- STScnVertex
//-------------------

//These MACROs are useful for shader code.
#define ScnVertex_IDX_x         0u
#define ScnVertex_IDX_y         4u
#define ScnVertex_IDX_color     8u
#define ScnVertex_SZ            12u

#define STScnVertex_Zero     { 0.f, 0.f, STScnColor8_Zero }

//Vertex without texture
typedef struct STScnVertex_ {
    ScnFLOAT    x;
    ScnFLOAT    y;
    STScnColor8 color;
} STScnVertex;

//--------------------
//-- STScnVertexTex
//--------------------

//These MACROs are useful for shader code.
#define ScnVertexTex_IDX_x      0u
#define ScnVertexTex_IDX_y      4u
#define ScnVertexTex_IDX_color  8u
#define ScnVertexTex_IDX_tex_x  12u
#define ScnVertexTex_IDX_tex_y  16u
#define ScnVertexTex_SZ         20u

#define STScnVertexTex_Zero     { 0.f, 0.f, STScnColor8_Zero, STScnPoint_Zero }

//Vertex with one texture
typedef struct STScnVertexTex_ {
    ScnFLOAT    x;
    ScnFLOAT    y;
    STScnColor8 color;
    STScnPoint  tex;
} STScnVertexTex;


//--------------------
//-- STScnVertexTex2
//--------------------

//These MACROs are useful for shader code.
#define ScnVertexTex2_IDX_x       0u
#define ScnVertexTex2_IDX_y       4u
#define ScnVertexTex2_IDX_color   8u
#define ScnVertexTex2_IDX_tex_x   12u
#define ScnVertexTex2_IDX_tex_y   16u
#define ScnVertexTex2_IDX_tex2_x  20u
#define ScnVertexTex2_IDX_tex2_y  24u
#define ScnVertexTex2_SZ          28u

#define STScnVertexTex2_Zero     { 0.f, 0.f, STScnColor8_Zero, STScnPoint_Zero, STScnPoint_Zero }

//Vertex with 2 textures
typedef struct STScnVertexTex2_ {
    ScnFLOAT    x;
    ScnFLOAT    y;
    STScnColor8 color;
    STScnPoint  tex;
    STScnPoint  tex2;
} STScnVertexTex2;

//--------------------
//-- STScnVertexTex3
//--------------------

//These MACROs are useful for shader code.
#define ScnVertexTex3_IDX_x         0u
#define ScnVertexTex3_IDX_y         4u
#define ScnVertexTex3_IDX_color     8u
#define ScnVertexTex3_IDX_tex_x     12u
#define ScnVertexTex3_IDX_tex_y     16u
#define ScnVertexTex3_IDX_tex2_x    20u
#define ScnVertexTex3_IDX_tex2_y    24u
#define ScnVertexTex3_IDX_tex3_x    28u
#define ScnVertexTex3_IDX_tex3_y    32u
#define ScnVertexTex3_SZ            36u

#define STScnVertexTex3_Zero     { 0.f, 0.f, STScnColor8_Zero, STScnPoint_Zero, STScnPoint_Zero, STScnPoint_Zero }

//Vertex with 3 textures
typedef struct STScnVertexTex3_ {
    ScnFLOAT    x;
    ScnFLOAT    y;
    STScnColor8 color;
    STScnPoint  tex;
    STScnPoint  tex2;
    STScnPoint  tex3;
} STScnVertexTex3;


//

//-------------------
//-- STScnVertexF
//-------------------

//These MACROs are useful for shader code.
#define ScnVertexF_IDX_x        0u
#define ScnVertexF_IDX_y        4u
#define ScnVertexF_IDX_color_r  8u
#define ScnVertexF_IDX_color_g  12u
#define ScnVertexF_IDX_color_b  16u
#define ScnVertexF_IDX_color_a  20u
#define ScnVertexF_SZ           24u

//Vertex without texture
typedef struct STScnVertexF_ {
    ScnFLOAT    x;
    ScnFLOAT    y;
    STScnColor  color;
} STScnVertexF;

//--------------------
//-- STScnVertexTexF
//--------------------
//
//These MACROs are useful for shader code.
#define ScnVertexTexF_IDX_x         0u
#define ScnVertexTexF_IDX_y         4u
#define ScnVertexTexF_IDX_color_r   8u
#define ScnVertexTexF_IDX_color_g   12u
#define ScnVertexTexF_IDX_color_b   16u
#define ScnVertexTexF_IDX_color_a   20u
#define ScnVertexTexF_IDX_tex_x     24u
#define ScnVertexTexF_IDX_tex_y     28u
#define ScnVertexTexF_SZ            32u

//Vertex with one texture
typedef struct STScnVertexTexF_ {
    ScnFLOAT    x;
    ScnFLOAT    y;
    STScnColor  color;
    STScnPoint  tex;
} STScnVertexTexF;


//--------------------
//-- STScnVertexTex2F
//--------------------

//These MACROs are useful for shader code.
#define ScnVertexTex2F_IDX_x        0u
#define ScnVertexTex2F_IDX_y        4u
#define ScnVertexTex2F_IDX_color_r  8u
#define ScnVertexTex2F_IDX_color_g  12u
#define ScnVertexTex2F_IDX_color_b  16u
#define ScnVertexTex2F_IDX_color_a  20u
#define ScnVertexTex2F_IDX_tex_x    24u
#define ScnVertexTex2F_IDX_tex_y    28u
#define ScnVertexTex2F_IDX_tex2_x   32u
#define ScnVertexTex2F_IDX_tex2_y   36u
#define ScnVertexTex2F_SZ           40u

//Vertex with 2 textures
typedef struct STScnVertexTex2F_ {
    ScnFLOAT    x;
    ScnFLOAT    y;
    STScnColor  color;
    STScnPoint  tex;
    STScnPoint  tex2;
} STScnVertexTex2F;


//--------------------
//-- STScnVertexTex3F
//--------------------

//These MACROs are useful for shader code.
#define ScnVertexTex3F_IDX_x        0u
#define ScnVertexTex3F_IDX_y        4u
#define ScnVertexTex3F_IDX_color_r  8u
#define ScnVertexTex3F_IDX_color_g  12u
#define ScnVertexTex3F_IDX_color_b  16u
#define ScnVertexTex3F_IDX_color_a  20u
#define ScnVertexTex3F_IDX_tex_x    24u
#define ScnVertexTex3F_IDX_tex_y    28u
#define ScnVertexTex3F_IDX_tex2_x   32u
#define ScnVertexTex3F_IDX_tex2_y   36u
#define ScnVertexTex3F_IDX_tex3_x   40u
#define ScnVertexTex3F_IDX_tex3_y   44u
#define ScnVertexTex3F_SZ           48u

//Vertex with 3 textures
typedef struct STScnVertexTex3F_ {
    ScnFLOAT    x;
    ScnFLOAT    y;
    STScnColor  color;
    STScnPoint  tex;
    STScnPoint  tex2;
    STScnPoint  tex3;
} STScnVertexTex3F;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnVertices_h */
