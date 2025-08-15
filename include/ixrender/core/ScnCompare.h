//
//  ScnCompare.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnCompare_h
#define ScnCompare_h

#include "ixrender/ixtli-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

//Signature of methods used to compare two structs or variables. Used for ordered arrays.
//Returns:  - zero if 'data1 == data2'
//          - a positive value if 'data1 > data2'
//          - a negative value otherwise ('data1 < data2' or invalid input)
typedef ScnSI32 (*ScnCompareFunc)(const void* data1, const void* data2, const ScnUI32 dataSz);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnCompare_h */
