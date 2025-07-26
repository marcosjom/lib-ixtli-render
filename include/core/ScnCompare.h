//
//  ScnCompare.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnCompare_h
#define ScnCompare_h

#include "ixtli-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ENCompareMode_ {
    ENCompareMode_Equal = 0,
    ENCompareMode_Lower,
    ENCompareMode_LowerOrEqual,
    ENCompareMode_Greater,
    ENCompareMode_GreaterOrEqual
} ENCompareMode;

typedef ScnBOOL (*NBCompareFunc)(const ENCompareMode mode, const void* data1, const void* data2, const ScnUI32 dataSz);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnCompare_h */
