//
//  ScnVertices.metal
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/type/ScnBitmapProps.h"
#include "ixrender/gpu/ScnGpuFramebuffProps.h"
#include "ixrender/gpu/ScnGpuModelProps2d.h"
#include "ixrender/scene/ScnVertices.h"


#include <metal_stdlib>
#include <metal_logging>
//#include <simd/simd.h>
using namespace metal;

// STScnVertex2D

struct RasterizerData {
    float4 position [[position]];
    float4 color;
};

vertex RasterizerData
ixtliVertexShader(constant STScnGpuFramebuffProps *fbProps [[buffer(0)]]
             , constant STScnGpuModelProps2d *mdlProps [[buffer(1)]]
             , constant STScnVertex2D *verts [[buffer(2)]]
             , uint iVert [[vertex_id]]
             )
{
    RasterizerData out;
    STScnVertex2D v = verts[iVert];
    //pos
    const float x   = (mdlProps->matrix.e00 * v.x) + (mdlProps->matrix.e01 * v.y) + mdlProps->matrix.e02;
    const float y   = (mdlProps->matrix.e10 * v.x) + (mdlProps->matrix.e11 * v.y) + mdlProps->matrix.e12;
    const float xRel= (fbProps->ortho.x.max == fbProps->ortho.x.min ? fbProps->ortho.x.max : (x - fbProps->ortho.x.min) / (fbProps->ortho.x.max - fbProps->ortho.x.min));
    const float yRel= (fbProps->ortho.y.max == fbProps->ortho.y.min ? fbProps->ortho.y.max : (y - fbProps->ortho.y.min) / (fbProps->ortho.y.max - fbProps->ortho.y.min));
    out.position.x  = -1.f + (xRel * 2.f);
    out.position.y  = -1.f + (yRel * 2.f);
    out.position.z  = 0.0;
    out.position.w  = 1.0;
    //
    //metal::os_log_default.log_info("viewport(%u, %u)(+%u, +%u)", fbProps->viewport.x, fbProps->viewport.y, fbProps->viewport.width, fbProps->viewport.height);
    //metal::os_log_default.log_info("mdlProps c8(%u, %u, %u, %u) m(%.1f, %.1f, %.1f)-(%.1f, %.1f, %.1f)", mdlProps->c8.r, mdlProps->c8.g, mdlProps->c8.b, mdlProps->c8.a, mdlProps->matrix._m00, mdlProps->matrix._m01, mdlProps->matrix._m02, mdlProps->matrix._m10, mdlProps->matrix._m11, mdlProps->matrix._m12);
    //metal::os_log_default.log_info("v[%u] = (%.1f, %.1f) c8(%u, %u, %u, %u)", iVert, v.x, v.y, v.color.r, v.color.g, v.color.b, v.color.a);
    //color
    out.color.r = (ScnFLOAT)v.color.r * (ScnFLOAT)mdlProps->c8.r / (255.f * 255.f);
    out.color.g = (ScnFLOAT)v.color.g * (ScnFLOAT)mdlProps->c8.g / (255.f * 255.f);
    out.color.b = (ScnFLOAT)v.color.b * (ScnFLOAT)mdlProps->c8.b / (255.f * 255.f);
    out.color.a = (ScnFLOAT)v.color.a * (ScnFLOAT)mdlProps->c8.a / (255.f * 255.f);
    //metal::os_log_default.log_info("v[%u] = (%.1f, %.1f) c(%.1f, %.1f, %.1f, %.1f)", iVert, out.position.x, out.position.y, out.color.r, out.color.g, out.color.b, out.color.a);
    return out;
}

fragment float4 ixtliFragmentShader(RasterizerData in [[stage_in]]) {
    return in.color;
}

// STScnVertex2DTex

struct RasterizerDataTex {
    float4  position [[position]];
    float4  color;
    float2  tex0Coord;
    ScnUI8  tex0Fmt;     //ENScnBitmapColor
};

vertex RasterizerDataTex
ixtliVertexShaderTex(constant STScnGpuFramebuffProps *fbProps [[buffer(0)]]
             , constant STScnGpuModelProps2d *mdlProps [[buffer(1)]]
             , constant STScnVertex2DTex *verts [[buffer(2)]]
             , uint iVert [[vertex_id]]
             )
{
    RasterizerDataTex out;
    STScnVertex2DTex v = verts[iVert];
    //pos
    const float x   = (mdlProps->matrix.e00 * v.x) + (mdlProps->matrix.e01 * v.y) + mdlProps->matrix.e02;
    const float y   = (mdlProps->matrix.e10 * v.x) + (mdlProps->matrix.e11 * v.y) + mdlProps->matrix.e12;
    const float xRel= (fbProps->ortho.x.max == fbProps->ortho.x.min ? fbProps->ortho.x.max : (x - fbProps->ortho.x.min) / (fbProps->ortho.x.max - fbProps->ortho.x.min));
    const float yRel= (fbProps->ortho.y.max == fbProps->ortho.y.min ? fbProps->ortho.y.max : (y - fbProps->ortho.y.min) / (fbProps->ortho.y.max - fbProps->ortho.y.min));
    out.position.x  = -1.f + (xRel * 2.f);
    out.position.y  = -1.f + (yRel * 2.f);
    out.position.z  = 0.0;
    out.position.w  = 1.0;
    //metal::os_log_default.log_info("viewport(%u, %u)(+%u, +%u)", fbProps->viewport.x, fbProps->viewport.y, fbProps->viewport.width, fbProps->viewport.height);
    //metal::os_log_default.log_info("mdlProps c8(%u, %u, %u, %u) m(%.1f, %.1f, %.1f)-(%.1f, %.1f, %.1f)", mdlProps->c8.r, mdlProps->c8.g, mdlProps->c8.b, mdlProps->c8.a, mdlProps->matrix._m00, mdlProps->matrix._m01, mdlProps->matrix._m02, mdlProps->matrix._m10, mdlProps->matrix._m11, mdlProps->matrix._m12);
    //metal::os_log_default.log_info("v[%u] = (%.1f, %.1f) c8(%u, %u, %u, %u)", iVert, v.x, v.y, v.color.r, v.color.g, v.color.b, v.color.a);
    //color
    out.color.r = (ScnFLOAT)v.color.r * (ScnFLOAT)mdlProps->c8.r / (255.f * 255.f);
    out.color.g = (ScnFLOAT)v.color.g * (ScnFLOAT)mdlProps->c8.g / (255.f * 255.f);
    out.color.b = (ScnFLOAT)v.color.b * (ScnFLOAT)mdlProps->c8.b / (255.f * 255.f);
    out.color.a = (ScnFLOAT)v.color.a * (ScnFLOAT)mdlProps->c8.a / (255.f * 255.f);
    //tex
    out.tex0Coord.x  = v.tex.x;
    out.tex0Coord.y  = v.tex.y;
    out.tex0Fmt      = mdlProps->texs.fmts[0];
    //metal::os_log_default.log_info("v[%u] = (%.1f, %.1f) c(%.1f, %.1f, %.1f, %.1f)", iVert, out.position.x, out.position.y, out.color.r, out.color.g, out.color.b, out.color.a);
    return out;
}

fragment float4 ixtliFragmentShaderTex(
                                       RasterizerDataTex in [[stage_in]]
                                       , texture2d<float> tex0 [[ texture(0) ]]
                                       , sampler smplr0 [[sampler(0)]]
                                       )
{
    float4 colorSample = tex0.sample(smplr0, in.tex0Coord);
    switch(in.tex0Fmt){
        case ENScnBitmapColor_GRAY8:
            //Convert this Red format in Metal to Gray+Alpha
            colorSample.a = 255u;
            colorSample.g = colorSample.b = colorSample.r;
            break;
        case ENScnBitmapColor_GRAYALPHA8:
            //Convert this Red+Green format in Metal to Gray+Alpha
            colorSample.a = colorSample.g;
            colorSample.g = colorSample.b = colorSample.r;
            break;
        default:
            break;
    }
    return in.color * colorSample;
}


// STScnVertex2DTex2

struct RasterizerDataTex2 {
    float4  position [[position]];
    float4  color;
    float2  tex0Coord;
    float2  tex1Coord;
    ScnUI8  tex0Fmt;     //ENScnBitmapColor
    ScnUI8  tex1Fmt;    //ENScnBitmapColor
};

vertex RasterizerDataTex2
ixtliVertexShaderTex2(constant STScnGpuFramebuffProps *fbProps [[buffer(0)]]
             , constant STScnGpuModelProps2d *mdlProps [[buffer(1)]]
             , constant STScnVertex2DTex2 *verts [[buffer(2)]]
             , uint iVert [[vertex_id]]
             )
{
    RasterizerDataTex2 out;
    STScnVertex2DTex2 v = verts[iVert];
    //pos
    const float x   = (mdlProps->matrix.e00 * v.x) + (mdlProps->matrix.e01 * v.y) + mdlProps->matrix.e02;
    const float y   = (mdlProps->matrix.e10 * v.x) + (mdlProps->matrix.e11 * v.y) + mdlProps->matrix.e12;
    const float xRel= (fbProps->ortho.x.max == fbProps->ortho.x.min ? fbProps->ortho.x.max : (x - fbProps->ortho.x.min) / (fbProps->ortho.x.max - fbProps->ortho.x.min));
    const float yRel= (fbProps->ortho.y.max == fbProps->ortho.y.min ? fbProps->ortho.y.max : (y - fbProps->ortho.y.min) / (fbProps->ortho.y.max - fbProps->ortho.y.min));
    out.position.x  = -1.f + (xRel * 2.f);
    out.position.y  = -1.f + (yRel * 2.f);
    out.position.z  = 0.0;
    out.position.w  = 1.0;
    //metal::os_log_default.log_info("viewport(%u, %u)(+%u, +%u)", fbProps->viewport.x, fbProps->viewport.y, fbProps->viewport.width, fbProps->viewport.height);
    //metal::os_log_default.log_info("mdlProps c8(%u, %u, %u, %u) m(%.1f, %.1f, %.1f)-(%.1f, %.1f, %.1f)", mdlProps->c8.r, mdlProps->c8.g, mdlProps->c8.b, mdlProps->c8.a, mdlProps->matrix._m00, mdlProps->matrix._m01, mdlProps->matrix._m02, mdlProps->matrix._m10, mdlProps->matrix._m11, mdlProps->matrix._m12);
    //metal::os_log_default.log_info("v[%u] = (%.1f, %.1f) c8(%u, %u, %u, %u)", iVert, v.x, v.y, v.color.r, v.color.g, v.color.b, v.color.a);
    //color
    out.color.r = (ScnFLOAT)v.color.r * (ScnFLOAT)mdlProps->c8.r / (255.f * 255.f);
    out.color.g = (ScnFLOAT)v.color.g * (ScnFLOAT)mdlProps->c8.g / (255.f * 255.f);
    out.color.b = (ScnFLOAT)v.color.b * (ScnFLOAT)mdlProps->c8.b / (255.f * 255.f);
    out.color.a = (ScnFLOAT)v.color.a * (ScnFLOAT)mdlProps->c8.a / (255.f * 255.f);
    //tex
    out.tex0Coord.x  = v.tex.x;
    out.tex0Coord.y  = v.tex.y;
    out.tex1Coord.x = v.tex2.x;
    out.tex1Coord.y = v.tex2.y;
    out.tex0Fmt      = mdlProps->texs.fmts[0];
    out.tex1Fmt     = mdlProps->texs.fmts[1];
    //metal::os_log_default.log_info("v[%u] = (%.1f, %.1f) c(%.1f, %.1f, %.1f, %.1f)", iVert, out.position.x, out.position.y, out.color.r, out.color.g, out.color.b, out.color.a);
    return out;
}

fragment float4 ixtliFragmentShaderTex2(
                                        RasterizerDataTex2 in [[stage_in]]
                                        , texture2d<float> tex0 [[ texture(0) ]]
                                        , texture2d<float> tex1 [[ texture(1) ]]
                                        , sampler smplr0 [[sampler(0)]]
                                        , sampler smplr1 [[sampler(1)]]
                                       )
{
    float4 colorSample = tex0.sample(smplr0, in.tex0Coord);
    switch(in.tex0Fmt){
        case ENScnBitmapColor_GRAY8:
            //Convert this Red format in Metal to Gray+Alpha
            colorSample.a = 255u;
            colorSample.g = colorSample.b = colorSample.r;
            break;
        case ENScnBitmapColor_GRAYALPHA8:
            //Convert this Red+Green format in Metal to Gray+Alpha
            colorSample.a = colorSample.g;
            colorSample.g = colorSample.b = colorSample.r;
            break;
        default:
            break;
    }
    float4 colorSample1 = tex1.sample(smplr1, in.tex0Coord);
    switch(in.tex1Fmt){
        case ENScnBitmapColor_GRAY8:
            //Convert this Red format in Metal to Gray+Alpha
            colorSample1.a = 255u;
            colorSample1.g = colorSample1.b = colorSample1.r;
            break;
        case ENScnBitmapColor_GRAYALPHA8:
            //Convert this Red+Green format in Metal to Gray+Alpha
            colorSample1.a = colorSample1.g;
            colorSample1.g = colorSample1.b = colorSample1.r;
            break;
        default:
            break;
    }
    return in.color * colorSample;
}

// STScnVertex2DTex3

struct RasterizerDataTex3 {
    float4  position [[position]];
    float4  color;
    float2  tex0Coord;
    float2  tex1Coord;
    float2  tex2Coord;
    ScnUI8  tex0Fmt;    //ENScnBitmapColor
    ScnUI8  tex1Fmt;    //ENScnBitmapColor
    ScnUI8  tex2Fmt;    //ENScnBitmapColor
};

vertex RasterizerDataTex3
ixtliVertexShaderTex3(constant STScnGpuFramebuffProps *fbProps [[buffer(0)]]
             , constant STScnGpuModelProps2d *mdlProps [[buffer(1)]]
             , constant STScnVertex2DTex3 *verts [[buffer(2)]]
             , uint iVert [[vertex_id]]
             )
{
    RasterizerDataTex3 out;
    STScnVertex2DTex3 v = verts[iVert];
    //pos
    const float x   = (mdlProps->matrix.e00 * v.x) + (mdlProps->matrix.e01 * v.y) + mdlProps->matrix.e02;
    const float y   = (mdlProps->matrix.e10 * v.x) + (mdlProps->matrix.e11 * v.y) + mdlProps->matrix.e12;
    const float xRel= (fbProps->ortho.x.max == fbProps->ortho.x.min ? fbProps->ortho.x.max : (x - fbProps->ortho.x.min) / (fbProps->ortho.x.max - fbProps->ortho.x.min));
    const float yRel= (fbProps->ortho.y.max == fbProps->ortho.y.min ? fbProps->ortho.y.max : (y - fbProps->ortho.y.min) / (fbProps->ortho.y.max - fbProps->ortho.y.min));
    out.position.x  = -1.f + (xRel * 2.f);
    out.position.y  = -1.f + (yRel * 2.f);
    out.position.z  = 0.0;
    out.position.w  = 1.0;
    //metal::os_log_default.log_info("viewport(%u, %u)(+%u, +%u)", fbProps->viewport.x, fbProps->viewport.y, fbProps->viewport.width, fbProps->viewport.height);
    //metal::os_log_default.log_info("mdlProps c8(%u, %u, %u, %u) m(%.1f, %.1f, %.1f)-(%.1f, %.1f, %.1f)", mdlProps->c8.r, mdlProps->c8.g, mdlProps->c8.b, mdlProps->c8.a, mdlProps->matrix._m00, mdlProps->matrix._m01, mdlProps->matrix._m02, mdlProps->matrix._m10, mdlProps->matrix._m11, mdlProps->matrix._m12);
    //metal::os_log_default.log_info("v[%u] = (%.1f, %.1f) c8(%u, %u, %u, %u)", iVert, v.x, v.y, v.color.r, v.color.g, v.color.b, v.color.a);
    //color
    out.color.r = (ScnFLOAT)v.color.r * (ScnFLOAT)mdlProps->c8.r / (255.f * 255.f);
    out.color.g = (ScnFLOAT)v.color.g * (ScnFLOAT)mdlProps->c8.g / (255.f * 255.f);
    out.color.b = (ScnFLOAT)v.color.b * (ScnFLOAT)mdlProps->c8.b / (255.f * 255.f);
    out.color.a = (ScnFLOAT)v.color.a * (ScnFLOAT)mdlProps->c8.a / (255.f * 255.f);
    //tex
    out.tex0Coord.x  = v.tex.x;
    out.tex0Coord.y  = v.tex.y;
    out.tex1Coord.x = v.tex2.x;
    out.tex1Coord.y = v.tex2.y;
    out.tex2Coord.x = v.tex3.x;
    out.tex2Coord.y = v.tex3.y;
    out.tex0Fmt      = mdlProps->texs.fmts[0];
    out.tex1Fmt     = mdlProps->texs.fmts[1];
    //metal::os_log_default.log_info("v[%u] = (%.1f, %.1f) c(%.1f, %.1f, %.1f, %.1f)", iVert, out.position.x, out.position.y, out.color.r, out.color.g, out.color.b, out.color.a);
    return out;
}

fragment float4 ixtliFragmentShaderTex3(
                                        RasterizerDataTex3 in [[stage_in]]
                                        , texture2d<float> tex0 [[ texture(0) ]]
                                        , texture2d<float> tex1 [[ texture(1) ]]
                                        , texture2d<float> tex2 [[ texture(2) ]]
                                        , sampler smplr0 [[sampler(0)]]
                                        , sampler smplr1 [[sampler(1)]]
                                        , sampler smplr2 [[sampler(2)]]
                                       )
{
    float4 colorSample = tex0.sample(smplr0, in.tex0Coord);
    switch(in.tex0Fmt){
        case ENScnBitmapColor_GRAY8:
            //Convert this Red format in Metal to Gray+Alpha
            colorSample.a = 255u;
            colorSample.g = colorSample.b = colorSample.r;
            break;
        case ENScnBitmapColor_GRAYALPHA8:
            //Convert this Red+Green format in Metal to Gray+Alpha
            colorSample.a = colorSample.g;
            colorSample.g = colorSample.b = colorSample.r;
            break;
        default:
            break;
    }
    float4 colorSample1 = tex1.sample(smplr1, in.tex0Coord);
    switch(in.tex1Fmt){
        case ENScnBitmapColor_GRAY8:
            //Convert this Red format in Metal to Gray+Alpha
            colorSample1.a = 255u;
            colorSample1.g = colorSample1.b = colorSample1.r;
            break;
        case ENScnBitmapColor_GRAYALPHA8:
            //Convert this Red+Green format in Metal to Gray+Alpha
            colorSample1.a = colorSample1.g;
            colorSample1.g = colorSample1.b = colorSample1.r;
            break;
        default:
            break;
    }
    float4 colorSample2 = tex2.sample(smplr2, in.tex0Coord);
    switch(in.tex2Fmt){
        case ENScnBitmapColor_GRAY8:
            //Convert this Red format in Metal to Gray+Alpha
            colorSample2.a = 255u;
            colorSample2.g = colorSample2.b = colorSample2.r;
            break;
        case ENScnBitmapColor_GRAYALPHA8:
            //Convert this Red+Green format in Metal to Gray+Alpha
            colorSample2.a = colorSample2.g;
            colorSample2.g = colorSample2.b = colorSample2.r;
            break;
        default:
            break;
    }
    return in.color * colorSample;
}

