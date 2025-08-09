//
//  ixtli-defs.c
//  ixtli-render
//
//  Created by Marcos Ortega on 9/8/25.
//

#include "ixrender/ixtli-defs.h"
#include <string.h> //memset, memcpy, memmove

void* ScnMemset(void* dst, const ScnSI32 ch, const ScnUI32 count){
    return memset(dst, ch, count);
}

void* ScnMemcpy(void* dst, const void* src, const ScnUI32 count){
    return memcpy(dst, src, count);
}

void* ScnMemmove(void* dst, const void* src, const ScnUI32 count){
    return memmove(dst, src, count);
}


#if defined(__OBJC__)
#   import <Foundation/Foundation.h> //for sin(), cos()
#else
#   include <math.h> //for sin(), cos()
#endif

//math.h

ScnFLOAT ScnSinf(ScnFLOAT num){
    return sinf(num);
}

ScnFLOAT ScnCosf(ScnFLOAT num){
    return cosf(num);
}
