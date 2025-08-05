//
//  ScnVertices.metal
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#include "ixrender/gpu/ScnGpuFramebuffProps.h"
#include "ixrender/gpu/ScnGpuModelProps2D.h"
#include "ixrender/scene/ScnVertices.h"

#include <metal_stdlib>
#include <metal_logging>
//#include <simd/simd.h>
using namespace metal;

/// A type that stores the vertex shader's output and serves as an input to the
/// fragment shader.
struct RasterizerData
{
    /// A 4D position in clip space from a vertex shader function.
    ///
    /// The `[[position]]` attribute indicates that the position is the vertex's
    /// clip-space position.
    float4 position [[position]];

    /// A color value, either for a vertex as an output from a vertex shader,
    /// or for a fragment as input to a fragment shader.
    ///
    /// As an input to a fragment shader, the rasterizer interpolates the color
    /// values between the triangle's vertices for each fragment because this
    /// member doesn't have a special attribute.
    float4 color;
};

/// A vertex shader that converts each input vertex from pixel coordinates to
/// clip-space coordinates.
///
/// The vertex shader doesn't modify the color values.
vertex RasterizerData
ixtliVertexShader(constant STScnGpuFramebuffProps *fbProps [[buffer(0)]]
             , constant STScnGpuModelProps2D *mdlProps [[buffer(1)]]
             , constant STScnVertex2D *verts [[buffer(2)]]
             , uint iVert [[vertex_id]]
             )
{
    RasterizerData out;

    STScnVertex2D v = verts[iVert];
    
    // Convert the position in pixel coordinates to clip-space by dividing the
    // pixel's coordinates by half the size of the viewport.
    const float x   = (mdlProps->matrix.e00 * v.x) + (mdlProps->matrix.e01 * v.y) + mdlProps->matrix.e02;
    const float y   = (mdlProps->matrix.e10 * v.x) + (mdlProps->matrix.e11 * v.y) + mdlProps->matrix.e12;
    const float xRel= (fbProps->ortho.x.max == fbProps->ortho.x.min ? fbProps->ortho.x.max : (x - fbProps->ortho.x.min) / (fbProps->ortho.x.max - fbProps->ortho.x.min));
    const float yRel= (fbProps->ortho.y.max == fbProps->ortho.y.min ? fbProps->ortho.y.max : (y - fbProps->ortho.y.min) / (fbProps->ortho.y.max - fbProps->ortho.y.min));
    
    out.position.x  = -1.f + (xRel * 2.f);
    out.position.y  = 1.f - (yRel * 2.f);
    out.position.z  = 0.0;
    out.position.w  = 1.0;
    
    //metal::os_log_default.log_info("viewport(%u, %u)(+%u, +%u)", fbProps->viewport.x, fbProps->viewport.y, fbProps->viewport.width, fbProps->viewport.height);
    //metal::os_log_default.log_info("mdlProps c8(%u, %u, %u, %u) m(%.1f, %.1f, %.1f)-(%.1f, %.1f, %.1f)", mdlProps->c8.r, mdlProps->c8.g, mdlProps->c8.b, mdlProps->c8.a, mdlProps->matrix._m00, mdlProps->matrix._m01, mdlProps->matrix._m02, mdlProps->matrix._m10, mdlProps->matrix._m11, mdlProps->matrix._m12);
    //metal::os_log_default.log_info("v[%u] = (%.1f, %.1f) c8(%u, %u, %u, %u)", iVert, v.x, v.y, v.color.r, v.color.g, v.color.b, v.color.a);

    // Pass the input color directly to the rasterizer.
    out.color.r = (ScnFLOAT)v.color.r * (ScnFLOAT)mdlProps->c8.r / (255.f * 255.f);
    out.color.g = (ScnFLOAT)v.color.g * (ScnFLOAT)mdlProps->c8.g / (255.f * 255.f);
    out.color.b = (ScnFLOAT)v.color.b * (ScnFLOAT)mdlProps->c8.b / (255.f * 255.f);
    out.color.a = (ScnFLOAT)v.color.a * (ScnFLOAT)mdlProps->c8.a / (255.f * 255.f);
    
    //metal::os_log_default.log_info("v[%u] = (%.1f, %.1f) c(%.1f, %.1f, %.1f, %.1f)", iVert, out.position.x, out.position.y, out.color.r, out.color.g, out.color.b, out.color.a);

    return out;
}

/// A basic fragment shader that returns the color data from the rasterizer
/// without modifying it.
fragment float4 ixtliFragmentShader(RasterizerData in [[stage_in]])
{
    // Return the color the rasterizer interpolates between the triangle's
    // three vertex colors.
    return in.color;
}

/// A type that stores the vertex shader's output and serves as an input to the
/// fragment shader.
struct RasterizerDataTex
{
    /// A 4D position in clip space from a vertex shader function.
    ///
    /// The `[[position]]` attribute indicates that the position is the vertex's
    /// clip-space position.
    float4 position [[position]];

    /// A color value, either for a vertex as an output from a vertex shader,
    /// or for a fragment as input to a fragment shader.
    ///
    /// As an input to a fragment shader, the rasterizer interpolates the color
    /// values between the triangle's vertices for each fragment because this
    /// member doesn't have a special attribute.
    float4 color;
    
    float2 textureCoordinate;
};

/// A vertex shader that converts each input vertex from pixel coordinates to
/// clip-space coordinates.
///
/// The vertex shader doesn't modify the color values.
vertex RasterizerDataTex
ixtliVertexShaderTex(constant STScnGpuFramebuffProps *fbProps [[buffer(0)]]
             , constant STScnGpuModelProps2D *mdlProps [[buffer(1)]]
             , constant STScnVertex2DTex *verts [[buffer(2)]]
             , uint iVert [[vertex_id]]
             )
{
    RasterizerDataTex out;

    STScnVertex2DTex v = verts[iVert];
    
    // Convert the position in pixel coordinates to clip-space by dividing the
    // pixel's coordinates by half the size of the viewport.
    const float x   = (mdlProps->matrix.e00 * v.x) + (mdlProps->matrix.e01 * v.y) + mdlProps->matrix.e02;
    const float y   = (mdlProps->matrix.e10 * v.x) + (mdlProps->matrix.e11 * v.y) + mdlProps->matrix.e12;
    const float xRel= (fbProps->ortho.x.max == fbProps->ortho.x.min ? fbProps->ortho.x.max : (x - fbProps->ortho.x.min) / (fbProps->ortho.x.max - fbProps->ortho.x.min));
    const float yRel= (fbProps->ortho.y.max == fbProps->ortho.y.min ? fbProps->ortho.y.max : (y - fbProps->ortho.y.min) / (fbProps->ortho.y.max - fbProps->ortho.y.min));
    
    out.position.x  = -1.f + (xRel * 2.f);
    out.position.y  = 1.f - (yRel * 2.f);
    out.position.z  = 0.0;
    out.position.w  = 1.0;
    
    //metal::os_log_default.log_info("viewport(%u, %u)(+%u, +%u)", fbProps->viewport.x, fbProps->viewport.y, fbProps->viewport.width, fbProps->viewport.height);
    //metal::os_log_default.log_info("mdlProps c8(%u, %u, %u, %u) m(%.1f, %.1f, %.1f)-(%.1f, %.1f, %.1f)", mdlProps->c8.r, mdlProps->c8.g, mdlProps->c8.b, mdlProps->c8.a, mdlProps->matrix._m00, mdlProps->matrix._m01, mdlProps->matrix._m02, mdlProps->matrix._m10, mdlProps->matrix._m11, mdlProps->matrix._m12);
    //metal::os_log_default.log_info("v[%u] = (%.1f, %.1f) c8(%u, %u, %u, %u)", iVert, v.x, v.y, v.color.r, v.color.g, v.color.b, v.color.a);

    // Pass the input color directly to the rasterizer.
    out.color.r = (ScnFLOAT)v.color.r * (ScnFLOAT)mdlProps->c8.r / (255.f * 255.f);
    out.color.g = (ScnFLOAT)v.color.g * (ScnFLOAT)mdlProps->c8.g / (255.f * 255.f);
    out.color.b = (ScnFLOAT)v.color.b * (ScnFLOAT)mdlProps->c8.b / (255.f * 255.f);
    out.color.a = (ScnFLOAT)v.color.a * (ScnFLOAT)mdlProps->c8.a / (255.f * 255.f);
    
    //
    
    out.textureCoordinate.x = v.tex.x;
    out.textureCoordinate.y = v.tex.y;
    
    //metal::os_log_default.log_info("v[%u] = (%.1f, %.1f) c(%.1f, %.1f, %.1f, %.1f)", iVert, out.position.x, out.position.y, out.color.r, out.color.g, out.color.b, out.color.a);

    return out;
}

fragment float4 ixtliFragmentShaderTex(
                                       RasterizerDataTex in [[stage_in]]
                                       , texture2d<float> tex0 [[ texture(0) ]]
                                       )
{
    constexpr sampler textureSampler (mag_filter::linear,
                                      min_filter::linear);
    
    // Sample the texture to obtain a color
    const float4 colorSample = tex0.sample(textureSampler, in.textureCoordinate);
   
    // Return the color the rasterizer interpolates between the triangle's
    // three vertex colors.
    return in.color * colorSample;
}


/// A vertex shader that converts each input vertex from pixel coordinates to
/// clip-space coordinates.
///
/// The vertex shader doesn't modify the color values.
vertex RasterizerData
ixtliVertexShaderTex2(constant STScnGpuFramebuffProps *fbProps [[buffer(0)]]
             , constant STScnGpuModelProps2D *mdlProps [[buffer(1)]]
             , constant STScnVertex2DTex2 *verts [[buffer(2)]]
             , uint iVert [[vertex_id]]
             )
{
    RasterizerData out;

    STScnVertex2DTex2 v = verts[iVert];
    
    // Convert the position in pixel coordinates to clip-space by dividing the
    // pixel's coordinates by half the size of the viewport.
    const float x   = (mdlProps->matrix.e00 * v.x) + (mdlProps->matrix.e01 * v.y) + mdlProps->matrix.e02;
    const float y   = (mdlProps->matrix.e10 * v.x) + (mdlProps->matrix.e11 * v.y) + mdlProps->matrix.e12;
    const float xRel= (fbProps->ortho.x.max == fbProps->ortho.x.min ? fbProps->ortho.x.max : (x - fbProps->ortho.x.min) / (fbProps->ortho.x.max - fbProps->ortho.x.min));
    const float yRel= (fbProps->ortho.y.max == fbProps->ortho.y.min ? fbProps->ortho.y.max : (y - fbProps->ortho.y.min) / (fbProps->ortho.y.max - fbProps->ortho.y.min));
    
    out.position.x  = -1.f + (xRel * 2.f);
    out.position.y  = 1.f - (yRel * 2.f);
    out.position.z  = 0.0;
    out.position.w  = 1.0;
    
    //metal::os_log_default.log_info("viewport(%u, %u)(+%u, +%u)", fbProps->viewport.x, fbProps->viewport.y, fbProps->viewport.width, fbProps->viewport.height);
    //metal::os_log_default.log_info("mdlProps c8(%u, %u, %u, %u) m(%.1f, %.1f, %.1f)-(%.1f, %.1f, %.1f)", mdlProps->c8.r, mdlProps->c8.g, mdlProps->c8.b, mdlProps->c8.a, mdlProps->matrix._m00, mdlProps->matrix._m01, mdlProps->matrix._m02, mdlProps->matrix._m10, mdlProps->matrix._m11, mdlProps->matrix._m12);
    //metal::os_log_default.log_info("v[%u] = (%.1f, %.1f) c8(%u, %u, %u, %u)", iVert, v.x, v.y, v.color.r, v.color.g, v.color.b, v.color.a);

    // Pass the input color directly to the rasterizer.
    out.color.r = (ScnFLOAT)v.color.r * (ScnFLOAT)mdlProps->c8.r / (255.f * 255.f);
    out.color.g = (ScnFLOAT)v.color.g * (ScnFLOAT)mdlProps->c8.g / (255.f * 255.f);
    out.color.b = (ScnFLOAT)v.color.b * (ScnFLOAT)mdlProps->c8.b / (255.f * 255.f);
    out.color.a = (ScnFLOAT)v.color.a * (ScnFLOAT)mdlProps->c8.a / (255.f * 255.f);
    
    //metal::os_log_default.log_info("v[%u] = (%.1f, %.1f) c(%.1f, %.1f, %.1f, %.1f)", iVert, out.position.x, out.position.y, out.color.r, out.color.g, out.color.b, out.color.a);

    return out;
}

fragment float4 ixtliFragmentShaderTex2(RasterizerData in [[stage_in]])
{
    // Return the color the rasterizer interpolates between the triangle's
    // three vertex colors.
    return in.color;
}



/// A vertex shader that converts each input vertex from pixel coordinates to
/// clip-space coordinates.
///
/// The vertex shader doesn't modify the color values.
vertex RasterizerData
ixtliVertexShaderTex3(constant STScnGpuFramebuffProps *fbProps [[buffer(0)]]
             , constant STScnGpuModelProps2D *mdlProps [[buffer(1)]]
             , constant STScnVertex2DTex3 *verts [[buffer(2)]]
             , uint iVert [[vertex_id]]
             )
{
    RasterizerData out;

    STScnVertex2DTex3 v = verts[iVert];
    
    // Convert the position in pixel coordinates to clip-space by dividing the
    // pixel's coordinates by half the size of the viewport.
    const float x   = (mdlProps->matrix.e00 * v.x) + (mdlProps->matrix.e01 * v.y) + mdlProps->matrix.e02;
    const float y   = (mdlProps->matrix.e10 * v.x) + (mdlProps->matrix.e11 * v.y) + mdlProps->matrix.e12;
    const float xRel= (fbProps->ortho.x.max == fbProps->ortho.x.min ? fbProps->ortho.x.max : (x - fbProps->ortho.x.min) / (fbProps->ortho.x.max - fbProps->ortho.x.min));
    const float yRel= (fbProps->ortho.y.max == fbProps->ortho.y.min ? fbProps->ortho.y.max : (y - fbProps->ortho.y.min) / (fbProps->ortho.y.max - fbProps->ortho.y.min));
    
    out.position.x  = -1.f + (xRel * 2.f);
    out.position.y  = 1.f - (yRel * 2.f);
    out.position.z  = 0.0;
    out.position.w  = 1.0;
    
    //metal::os_log_default.log_info("viewport(%u, %u)(+%u, +%u)", fbProps->viewport.x, fbProps->viewport.y, fbProps->viewport.width, fbProps->viewport.height);
    //metal::os_log_default.log_info("mdlProps c8(%u, %u, %u, %u) m(%.1f, %.1f, %.1f)-(%.1f, %.1f, %.1f)", mdlProps->c8.r, mdlProps->c8.g, mdlProps->c8.b, mdlProps->c8.a, mdlProps->matrix._m00, mdlProps->matrix._m01, mdlProps->matrix._m02, mdlProps->matrix._m10, mdlProps->matrix._m11, mdlProps->matrix._m12);
    //metal::os_log_default.log_info("v[%u] = (%.1f, %.1f) c8(%u, %u, %u, %u)", iVert, v.x, v.y, v.color.r, v.color.g, v.color.b, v.color.a);

    // Pass the input color directly to the rasterizer.
    out.color.r = (ScnFLOAT)v.color.r * (ScnFLOAT)mdlProps->c8.r / (255.f * 255.f);
    out.color.g = (ScnFLOAT)v.color.g * (ScnFLOAT)mdlProps->c8.g / (255.f * 255.f);
    out.color.b = (ScnFLOAT)v.color.b * (ScnFLOAT)mdlProps->c8.b / (255.f * 255.f);
    out.color.a = (ScnFLOAT)v.color.a * (ScnFLOAT)mdlProps->c8.a / (255.f * 255.f);
    
    //metal::os_log_default.log_info("v[%u] = (%.1f, %.1f) c(%.1f, %.1f, %.1f, %.1f)", iVert, out.position.x, out.position.y, out.color.r, out.color.g, out.color.b, out.color.a);

    return out;
}

fragment float4 ixtliFragmentShaderTex3(RasterizerData in [[stage_in]])
{
    // Return the color the rasterizer interpolates between the triangle's
    // three vertex colors.
    return in.color;
}

