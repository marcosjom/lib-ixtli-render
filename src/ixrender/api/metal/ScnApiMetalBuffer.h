//
//  ScnApiMetalBuffer.h
//  ixtli-render
//
//  Created by Marcos Ortega on 18/8/25.
//

#ifndef ScnApiMetalBuffer_h
#define ScnApiMetalBuffer_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnContext.h"
#include "ixrender/api/ScnApiItf.h"
//
#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>
#import <TargetConditionals.h>  //for TARGET_OS_* macros

#ifdef __cplusplus
extern "C" {
#endif

//STScnApiMetalBuffer

typedef struct STScnApiMetalBuffer {
    ScnContextRef   ctx;
    STScnApiItf     itf;
    id<MTLDevice>   dev;
    id<MTLBuffer>   buff;
} STScnApiMetalBuffer;

ScnBOOL ScnApiMetalBuffer_syncRanges_(id<MTLBuffer> buff, ScnMemElasticRef mem, const STScnRangeU* const rngs, const ScnUI32 rngsUse);

void    ScnApiMetalBuffer_free(void* data);
ScnBOOL ScnApiMetalBuffer_sync(void* data, ScnMemElasticRef mem, const STScnGpuBufferChanges* changes);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiMetalBuffer_h */
