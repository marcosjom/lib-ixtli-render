//
//  ScnSize.h
//  ixtli-render
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnSize_h
#define ScnSize_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnCompare.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STScnSize_Zero { 0.f, 0.f }

typedef struct STScnSize_ {
    ScnFLOAT    width;
    ScnFLOAT    height;
} STScnSize;

//

#define STScnSizeI_Zero { 0, 0 }

typedef struct STScnSizeI_ {
    ScnSI32    width;
    ScnSI32    height;
} STScnSizeI;

//

#define STScnSizeU_Zero { 0u, 0u }

typedef struct STScnSizeU_ {
    ScnUI32    width;
    ScnUI32    height;
} STScnSizeU;

//

#define STScnSizeI16_Zero { 0, 0 }

typedef struct STScnSizeI16_ {
    ScnSI16    width;
    ScnSI16    height;
} STScnSizeI16;

//

ScnBOOL ScnCompare_NBSize(const ENScnCompareMode mode, const void* data1, const void* data2, const ScnUI32 dataSz);


#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnSize_h */
