//
//  ScnRenderCfg.h
//  ixtli-render
//
//  Created by Marcos Ortega on 21/8/25.
//

#ifndef ScnRenderCfg_h
#define ScnRenderCfg_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/scene/ScnVertex.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnRenderVertsCfg

/** @struct STScnRenderVertsCfg
 *  @brief Render vertices allocation configuration.
 *  @note These hints provided by the user should define the efficiency of memory jumps/allocation while defining models, syncing buffers and rendering the scene.
 *  @var STScnRenderVertsCfg::idxsPerBlock
 *  Ammount of indices per block of memory allocated.
 *  @var STScnRenderVertsCfg::typesPerBlock
 *  Ammount of vertices per block of memory allocated.
 */

#define STScnRenderVertsCfg_Zero       { 0, ENScnVertexType_Count_Zeroes }

typedef struct STScnRenderVertsCfg {
    ScnUI32     idxsPerBlock;       //2048
    ScnUI32     typesPerBlock[ENScnVertexType_Count]; // { 256, 1024, 256, 256 }
} STScnRenderVertsCfg;

//STScnRenderMemCfg

/** @struct STScnRenderMemCfg
 *  @brief Render memory allocation configuration.
 *  @note These hints provided by the user should define the efficiency of memory allocation and jumps while defining models, syncing buffers and rendering.
 *  @var STScnRenderMemCfg::propsScnsPerBlock
 *  Ammount of scenes properties (viewport, ortho, ...) per block of memory allocated.
 *  @var STScnRenderMemCfg::propsMdlsPerBlock
 *  Ammount of models properties (matrix, color, etc...) per block of memory allocated.
 *  @var STScnRenderMemCfg::verts
 *  Vertices allocation config.
 */

#define STScnRenderMemCfg_Zero       { 0, 0, STScnRenderVertsCfg_Zero }

typedef struct STScnRenderMemCfg {
    ScnUI32     propsScnsPerBlock;  //32
    ScnUI32     propsMdlsPerBlock;  //128
    STScnRenderVertsCfg verts;
} STScnRenderMemCfg;

//STScnRenderCfg

/** @struct STScnRenderCfg
 *  @brief Render configuration.
 *  @note These hints provided by the user should define the efficiency of memory allocation and jumps while defining models, syncing buffers and rendering.
 *  @var STScnRenderCfg::mem
 *  Memory allocation config.
 */

#define STScnRenderCfg_Zero       { STScnRenderMemCfg_Zero }

typedef struct STScnRenderCfg {
    STScnRenderMemCfg   mem;
} STScnRenderCfg;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRenderCfg_h */
