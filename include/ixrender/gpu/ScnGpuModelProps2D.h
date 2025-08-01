//
//  ScnGpuModelProps2D.h
//  ixtli-render
//
//  Created by Marcos Ortega on 30/7/25.
//

#ifndef ScnGpuModelProps2D_h
#define ScnGpuModelProps2D_h

#include "ixrender/type/ScnColor.h"
#include "ixrender/type/ScnMatrix.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnModelProps2D

#define STScnGpuModelProps2D_Zero        { STScnColor8_Zero, STScnMatrix2D_Zero }
#define STScnGpuModelProps2D_Identity    { STScnColor8_255, STScnMatrix2D_Identity }

typedef struct STScnGpuModelProps2D {
    STScnColor8     c8;     //color
    STScnMatrix2D   matrix; //matrix
} STScnGpuModelProps2D;

#ifndef SNC_COMPILING_SHADER
SC_INLN STScnGpuModelProps2D ScnGpuModelProps2D_multiply(const STScnGpuModelProps2D* const obj, const STScnGpuModelProps2D* const other){
    return (STScnGpuModelProps2D){
        {
            (ScnUI32)obj->c8.c[0] * (ScnUI32)other->c8.c[0] / 255u
            , (ScnUI32)obj->c8.c[1] * (ScnUI32)other->c8.c[1] / 255u
            , (ScnUI32)obj->c8.c[2] * (ScnUI32)other->c8.c[2] / 255u
            , (ScnUI32)obj->c8.c[3] * (ScnUI32)other->c8.c[3] / 255u
        }
        , ScnMatrix2D_multiply(&obj->matrix, &other->matrix)
    };
}
#endif

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuModelProps2D_h */
