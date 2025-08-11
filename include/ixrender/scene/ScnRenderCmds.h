//
//  ScnRenderCmds.h
//  ixtli-render
//
//  Created by Marcos Ortega on 10/8/25.
//

#ifndef ScnRenderCmds_h
#define ScnRenderCmds_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnArray.h"
#include "ixrender/core/ScnArraySorted.h"
#include "ixrender/core/ScnMemElastic.h"
#include "ixrender/scene/ScnFramebuff.h"
#include "ixrender/scene/ScnVertexbuff.h"
#include "ixrender/scene/ScnTexture.h"
#include "ixrender/scene/ScnRenderCmd.h"
#include "ixrender/gpu/ScnGpuDeviceDesc.h"
#include "ixrender/gpu/ScnGpuFramebuffProps.h"
#include "ixrender/gpu/ScnGpuModelProps2d.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnRenderFbuffProps

#define STScnRenderFbuffProps_Zero   { STScnGpuFramebuffProps_Zero, STScnAbsPtr_Zero }

typedef struct STScnRenderFbuffProps {
    STScnGpuFramebuffProps  props;
    STScnAbsPtr             ptr;
} STScnRenderFbuffProps;

//STScnRenderFbuffState

typedef struct STScnRenderFbuffState {
    ScnContextRef       ctx;
    ScnFramebuffRef     fbuff;
    //stacks
    struct {
        ScnArrayStruct(props, STScnRenderFbuffProps);       //stack of props (viewport, ortho, ...)
        ScnArrayStruct(transforms, STScnGpuModelProps2d);   //stack of transformations (color, matrix, ...)
    } stacks;
    //active (last state, helps to reduce redundant cmds)
    struct {
        ScnVertexbuffRef vbuff;
        //texs
        struct {
            ScnTextureRef refs[ENScnGpuTextureIdx_Count];
            ScnUI32     changesSeq;
        } texs;
    } active;
} STScnRenderFbuffState;

void ScnRenderFbuffState_init(ScnContextRef ctx, STScnRenderFbuffState* obj);
void ScnRenderFbuffState_destroy(STScnRenderFbuffState* obj);
//

SC_INLN void ScnRenderFbuffState_resetActiveState(STScnRenderFbuffState* obj){
    ScnMemory_setZeroSt(obj->active);
}


SC_INLN void ScnRenderFbuffState_reset(STScnRenderFbuffState* obj){
    ScnArray_empty(&obj->stacks.props);
    ScnArray_empty(&obj->stacks.transforms);
    ScnRenderFbuffState_resetActiveState(obj);
}

SC_INLN ScnBOOL ScnRenderFbuffState_addProps(STScnRenderFbuffState* obj, const STScnRenderFbuffProps* const p){
    return ScnArray_addPtr(obj->ctx, &obj->stacks.props, p, STScnRenderFbuffProps) != NULL ? ScnTRUE : ScnFALSE;
}

SC_INLN ScnBOOL ScnRenderFbuffState_addTransform(STScnRenderFbuffState* obj, const STScnGpuModelProps2d* const t){
    return ScnArray_addPtr(obj->ctx, &obj->stacks.transforms, t, STScnGpuModelProps2d) != NULL ? ScnTRUE : ScnFALSE;
}

//ENScnRenderJobObjType

typedef enum ENScnRenderJobObjType {
    ENScnRenderJobObjType_Unknown = 0,
    ENScnRenderJobObjType_Buff,
    ENScnRenderJobObjType_Framebuff,
    ENScnRenderJobObjType_Vertexbuff,
    ENScnRenderJobObjType_Texture,
    //
    ENScnRenderJobObjType_Count
} ENScnRenderJobObjType;

//STScnRenderJobObj

typedef struct STScnRenderJobObj {
    ENScnRenderJobObjType   type;
    union {
        ScnObjRef           objRef;     //generic ref (compatible will all bellow it)
        ScnBufferRef        buff;
        ScnFramebuffRef     framebuff;
        ScnVertexbuffRef    vertexbuff;
        ScnTextureRef       texture;
    };
} STScnRenderJobObj;

void ScnRenderJobObj_init(STScnRenderJobObj* obj);
void ScnRenderJobObj_destroy(STScnRenderJobObj* obj);
//
ScnBOOL ScnCompare_ScnRenderJobObj(const ENScnCompareMode mode, const void* data1, const void* data2, const ScnUI32 dataSz);


//STScnRenderCmds

typedef struct STScnRenderCmds {
    ScnContextRef       ctx;
    ScnBOOL             isPrepared;
    //
    STScnGpuDeviceDesc  gpuDevDesc;
    ScnMemElasticRef    mPropsScns;     //buffer with scenes properties (viewport, ortho-box)
    ScnMemElasticRef    mPropsMdls;     //buffer with models properties (color, matrices)
    //
    ScnUI32             stackUse;       //effective use of the stack-array (the array is reused between renders)
    ScnArrayStruct(     stack, STScnRenderFbuffState);   //stack of framebuffers
    ScnArrayStruct(     cmds, STScnRenderCmd);           //cmds sequence
    ScnArraySortedStruct(objs, STScnRenderJobObj);      //unique references of objects using for thias job
} STScnRenderCmds;

void    ScnRenderCmds_init(ScnContextRef ctx, STScnRenderCmds* obj);
void    ScnRenderCmds_destroy(STScnRenderCmds* obj);
//
ScnBOOL ScnRenderCmds_prepare(STScnRenderCmds* obj, const STScnGpuDeviceDesc* gpuDevDesc, const ScnUI32 propsScnsPerBlock, const ScnUI32 propsMdlsPerBlock);
//
void    ScnRenderCmds_reset(STScnRenderCmds* obj);
ScnBOOL ScnRenderCmds_add(STScnRenderCmds* obj, const STScnRenderCmd* const cmd);
ScnBOOL ScnRenderCmds_addUsedObj(STScnRenderCmds* obj, const ENScnRenderJobObjType type, ScnObjRef* objRef, ScnBOOL* optDstIsFirstUse);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRenderCmds_h */
