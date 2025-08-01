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
vertexShader(constant STScnGpuFramebufferProps *fbProps [[buffer(0)]]
             , constant STScnGpuFramebufferProps *mdlProps [[buffer(1)]]
             , constant STScnVertex2D *verts [[buffer(2)]]
             , uint iVert [[vertex_id]]
             )
{
    RasterizerData out;

    STScnVertex2D v = verts[iVert];
    
    
    // Retrieve the 2D position in pixel coordinates.
    /*simd_float2 pixelSpacePosition = vertexData[vertexID].position.xy;

    // Retrieve the viewport's size by casting it to a 2D float value.
    simd_float2 viewportSize = simd_float2(*viewportSizePointer);*/

    // Convert the position in pixel coordinates to clip-space by dividing the
    // pixel's coordinates by half the size of the viewport.
    out.position.x = v.x / (fbProps->size.width / 2);
    out.position.y = v.y / (fbProps->size.height / 2);
    out.position.z = 0.0;
    out.position.w = 1.0;

    // Pass the input color directly to the rasterizer.
    out.color.r = 1.0f; //(ScnFLOAT)v.color.r / 255.f;
    out.color.g = 1.0f; //(ScnFLOAT)v.color.g / 255.f;
    out.color.b = 1.0f; //(ScnFLOAT)v.color.b / 255.f;
    out.color.a = 1.0f; //(ScnFLOAT)v.color.a / 255.f;
    
    return out;
}

/// A basic fragment shader that returns the color data from the rasterizer
/// without modifying it.
fragment float4 fragmentShader(RasterizerData in [[stage_in]])
{
    // Return the color the rasterizer interpolates between the triangle's
    // three vertex colors.
    return in.color;
}

