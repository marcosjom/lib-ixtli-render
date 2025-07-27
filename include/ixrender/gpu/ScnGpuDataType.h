//
//  ScnGpuTypes.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnGpuTypes_h
#define ScnGpuTypes_h

#include "ixrender/ixtli-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

//ENScnGpuDataType

typedef enum ENScnGpuDataType_ {
    ENScnGpuDataType_SI8 = 0,
    ENScnGpuDataType_UI8,
    ENScnGpuDataType_SI16,
    ENScnGpuDataType_UI16,
    ENScnGpuDataType_SI32,
    ENScnGpuDataType_UI32,
    ENScnGpuDataType_FLOAT32,
    ENScnGpuDataType_DOUBLE64,
    //Count
    ENScnGpuDataType_Count
} ENScnGpuDataType;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuTypes_h */
