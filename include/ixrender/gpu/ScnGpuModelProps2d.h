//
//  ScnGpuModelProps2d.h
//  ixtli-render
//
//  Created by Marcos Ortega on 30/7/25.
//

#ifndef ScnGpuModelProps2d_h
#define ScnGpuModelProps2d_h

#include "ixrender/type/ScnColor.h"
#include "ixrender/type/ScnMatrix.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnNode2dProps

/** @struct STScnGpuModelProps2d
 *  @brief Properties for the model rendering.
 *  @var STScnGpuModelProps2d::c8
 *  Color multiplier to be applied to the model vertices.
 *  @var STScnGpuModelProps2d::matrix
 *  Matrix multiplier to be applied to the model vertices.
 *  @var STScnGpuModelProps2d::texs
 *  Textures colors formats. Only provided/required if STScnGpuDeviceDesc::isTexFmtInfoRequired is !ScnFALSE.
 */

#define STScnGpuModelProps2d_Extra_Zero  { { 0u, 0u, 0u } }
#define STScnGpuModelProps2d_Zero        { STScnColor8_Zero, STScnMatrix2D_Zero, STScnGpuModelProps2d_Extra_Zero }
#define STScnGpuModelProps2d_Identity    { STScnColor8_255, STScnMatrix2D_Identity, STScnGpuModelProps2d_Extra_Zero }

typedef struct STScnGpuModelProps2d {
    STScnColor8     c8;     //color
    STScnMatrix2D   matrix; //matrix
    //textures info
    struct {
        ScnUI8 fmts[3]; //ENScnBitmapColor
    } texs;
} STScnGpuModelProps2d;

/**
 * @brief Multiplies two model properties: colors and matrices.
 * @param obj First property.
 * @param other Second property.
 * @return Multiplied model properties.
 */
#ifndef SNC_COMPILING_SHADER
SC_INLN STScnGpuModelProps2d ScnGpuModelProps2d_multiply(const STScnGpuModelProps2d* const obj, const STScnGpuModelProps2d* const other){
    return SCN_ST(STScnGpuModelProps2d,
        {
            { // color
                { //union
                    { //array
                        (ScnUI8)((ScnUI32)obj->c8.c[0] * (ScnUI32)other->c8.c[0] / 255u)
                        , (ScnUI8)((ScnUI32)obj->c8.c[1] * (ScnUI32)other->c8.c[1] / 255u)
                        , (ScnUI8)((ScnUI32)obj->c8.c[2] * (ScnUI32)other->c8.c[2] / 255u)
                        , (ScnUI8)((ScnUI32)obj->c8.c[3] * (ScnUI32)other->c8.c[3] / 255u)
                    }
                }
            }
            , ScnMatrix2D_multiply(&obj->matrix, &other->matrix)
            , STScnGpuModelProps2d_Extra_Zero
        }
    );
}
#endif

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuModelProps2d_h */
