//
//  ScnNode2d.h
//  ixtli-render
//
//  Created by Marcos Ortega on 02/8/25.
//

#ifndef ScnNode2d_h
#define ScnNode2d_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/type/ScnColor.h"
#include "ixrender/type/ScnPoint.h"
#include "ixrender/type/ScnSize.h"
#include "ixrender/scene/ScnTransform2D.h"
#include "ixrender/scene/ScnNode2dProps.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnNode2dRef

SCN_REF_STRUCT_METHODS_DEC(ScnNode2d)

//props

STScnNode2dProps    ScnNode2d_getProps(ScnNode2dRef ref);

//color

STScnColor8         ScnNode2d_getColor8(ScnNode2dRef ref);
void                ScnNode2d_setColor8(ScnNode2dRef ref, const STScnColor8 color);
void                ScnNode2d_setColorRGBA8(ScnNode2dRef ref, const ScnUI8 r, const ScnUI8 g, const ScnUI8 b, const ScnUI8 a);

//transform

STScnTransform2D    ScnNode2d_getTransform(ScnNode2dRef ref);
STScnPoint2D        ScnNode2d_getTranslate(ScnNode2dRef ref);
STScnSize2D         ScnNode2d_getScale(ScnNode2dRef ref);
ScnFLOAT            ScnNode2d_getRotDeg(ScnNode2dRef ref);
ScnFLOAT            ScnNode2d_getRotRad(ScnNode2dRef ref);
void                ScnNode2d_setTranslate(ScnNode2dRef ref, const STScnPoint2D pos);
void                ScnNode2d_setTranslateXY(ScnNode2dRef ref, const ScnFLOAT x, const ScnFLOAT y);
void                ScnNode2d_setScale(ScnNode2dRef ref, const STScnSize2D s);
void                ScnNode2d_setScaleWH(ScnNode2dRef ref, const ScnFLOAT sw, const ScnFLOAT sh);
void                ScnNode2d_setRotDeg(ScnNode2dRef ref, const ScnFLOAT deg);
void                ScnNode2d_setRotRad(ScnNode2dRef ref, const ScnFLOAT rad);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnNode2d_h */
