//
//  ScnGpuTransform.h
//  ixtli-render
//
//  Created by Marcos Ortega on 30/7/25.
//

#ifndef ScnGpuTransform_h
#define ScnGpuTransform_h

#include "ixrender/type/ScnColor.h"
#include "ixrender/type/ScnMatrix.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnModelProps

#define STScnGpuTransform_Zero        { STScnColor8_Zero, STScnMatrix_Zero }
#define STScnGpuTransform_Identity    { STScnColor8_255, STScnMatrix_Identity }

typedef struct STScnGpuTransform_ {
    STScnColor8     c8;     //color
    STScnMatrix     matrix; //matrix
} STScnGpuTransform;

SC_INLN STScnGpuTransform ScnGpuTransform_multiply(const STScnGpuTransform* const obj, const STScnGpuTransform* const other){
    return (STScnGpuTransform){
        {
            (ScnUI32)obj->c8.c[0] * (ScnUI32)other->c8.c[0] / 255u
            , (ScnUI32)obj->c8.c[1] * (ScnUI32)other->c8.c[1] / 255u
            , (ScnUI32)obj->c8.c[2] * (ScnUI32)other->c8.c[2] / 255u
            , (ScnUI32)obj->c8.c[3] * (ScnUI32)other->c8.c[3] / 255u
        }
        , ScnMatrix_multiply(&obj->matrix, &other->matrix)
    };
}

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuTransform_h */
