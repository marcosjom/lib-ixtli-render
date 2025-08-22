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

/** @brief Compare function definition.
 *  @param data1 Left side value's pointer.
 *  @param data2 Right side value's pointer.
 *  @param dataSz Value's size in bytes.
 *  @return Zero if 'v1 == v2', a positive value if 'v1 > v2', a negative value otherwise (including invalid inputs).
 */
typedef ScnSI32 (*ScnCompareFunc)(const void* data1, const void* data2, const ScnUI32 dataSz);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnCompare_h */
