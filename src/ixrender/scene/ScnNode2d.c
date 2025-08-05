//
//  ScnNode2d.c
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 02/8/25.
//

#include "ixrender/scene/ScnNode2d.h"
#include "ixrender/core/ScnArray.h"

//STScnNode2dOpq

typedef struct STScnNode2dOpq {
    ScnContextRef       ctx;
    ScnMutexRef         mutex;
    //
    STScnNode2dProps     props;
} STScnNode2dOpq;

//

ScnSI32 ScnNode2d_getOpqSz(void){
    return (ScnSI32)sizeof(STScnNode2dOpq);
}

void ScnNode2d_initZeroedOpq(ScnContextRef ctx, void* obj) {
    STScnNode2dOpq* opq = (STScnNode2dOpq*)obj;
    //
    ScnContext_set(&opq->ctx, ctx);
    opq->mutex  = ScnContext_allocMutex(opq->ctx);
    //
    opq->props  = (STScnNode2dProps)STScnNode2dProps_Identity;
}

void ScnNode2d_destroyOpq(void* obj){
    STScnNode2dOpq* opq = (STScnNode2dOpq*)obj;
    ScnMutex_freeAndNullify(&opq->mutex);
    ScnContext_releaseAndNull(&opq->ctx);
}

//props

STScnNode2dProps ScnNode2d_getProps(ScnNode2dRef ref){
    STScnNode2dOpq* opq = (STScnNode2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->props;
}

//color

STScnColor8 ScnNode2d_getColor8(ScnNode2dRef ref){
    STScnNode2dOpq* opq = (STScnNode2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->props.c8;
}

void ScnNode2d_setColor8(ScnNode2dRef ref, const STScnColor8 color){
    STScnNode2dOpq* opq = (STScnNode2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.c8 = color;
}

void ScnNode2d_setColorRGBA8(ScnNode2dRef ref, const ScnUI8 r, const ScnUI8 g, const ScnUI8 b, const ScnUI8 a){
    STScnNode2dOpq* opq = (STScnNode2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.c8.r = r;
    opq->props.c8.g = g;
    opq->props.c8.b = b;
    opq->props.c8.a = a;
}

//transform

STScnTransform2D ScnNode2d_getTransform(ScnNode2dRef ref){
    STScnNode2dOpq* opq = (STScnNode2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->props.tform;
}

STScnPoint2D ScnNode2d_getTranslate(ScnNode2dRef ref){
    STScnNode2dOpq* opq = (STScnNode2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (STScnPoint2D){ opq->props.tform.tx, opq->props.tform.ty };
}

STScnSize2D ScnNode2d_getScale(ScnNode2dRef ref){
    STScnNode2dOpq* opq = (STScnNode2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return (STScnSize2D){ opq->props.tform.sx, opq->props.tform.sy };
}

ScnFLOAT ScnNode2d_getRotDeg(ScnNode2dRef ref){
    STScnNode2dOpq* opq = (STScnNode2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return opq->props.tform.deg;
}

ScnFLOAT ScnNode2d_getRotRad(ScnNode2dRef ref){
    STScnNode2dOpq* opq = (STScnNode2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    return DEG_2_RAD(opq->props.tform.deg);
}

void ScnNode2d_setTranslate(ScnNode2dRef ref, const STScnPoint2D pos){
    STScnNode2dOpq* opq = (STScnNode2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.tx = pos.x;
    opq->props.tform.ty = pos.y;
}

void ScnNode2d_setTranslateXY(ScnNode2dRef ref, const ScnFLOAT x, const ScnFLOAT y){
    STScnNode2dOpq* opq = (STScnNode2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.tx = x;
    opq->props.tform.ty = y;
}

void ScnNode2d_setScale(ScnNode2dRef ref, const STScnSize2D s){
    STScnNode2dOpq* opq = (STScnNode2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.sx = s.width;
    opq->props.tform.sy = s.height;
}

void ScnNode2d_setScaleWH(ScnNode2dRef ref, const ScnFLOAT sw, const ScnFLOAT sh){
    STScnNode2dOpq* opq = (STScnNode2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.sx = sw;
    opq->props.tform.sy = sh;
}

void ScnNode2d_setRotDeg(ScnNode2dRef ref, const ScnFLOAT deg){
    STScnNode2dOpq* opq = (STScnNode2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.deg = deg;
}

void ScnNode2d_setRotRad(ScnNode2dRef ref, const ScnFLOAT rad){
    STScnNode2dOpq* opq = (STScnNode2dOpq*)ScnSharedPtr_getOpq(ref.ptr);
    opq->props.tform.deg = RAD_2_DEG(rad);
}
