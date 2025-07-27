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

typedef enum ENScnCompareMode_ {
    ENScnCompareMode_Equal = 0,
    ENScnCompareMode_Lower,
    ENScnCompareMode_LowerOrEqual,
    ENScnCompareMode_Greater,
    ENScnCompareMode_GreaterOrEqual
} ENScnCompareMode;

typedef ScnBOOL (*ScnCompareFunc)(const ENScnCompareMode mode, const void* data1, const void* data2, const ScnUI32 dataSz);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnCompare_h */
