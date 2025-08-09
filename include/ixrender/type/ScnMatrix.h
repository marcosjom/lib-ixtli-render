//
//  ScnMatrix.h
//  ixtli-render
//
//  Created by Marcos Ortega on 30/7/25.
//

#ifndef ScnMatrix2D_h
#define ScnMatrix2D_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/type/ScnPoint.h"
#include "ixrender/type/ScnSize.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STScnMatrix2D_Zero        { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f }
#define STScnMatrix2D_Identity    { 1.f, 0.f, 0.f, 0.f, 1.f, 0.f }

typedef struct STScnMatrix2D {
    union {
        struct {
            //row 0
            ScnFLOAT e00;
            ScnFLOAT e01;
            ScnFLOAT e02;
            //row 1
            ScnFLOAT e10;
            ScnFLOAT e11;
            ScnFLOAT e12;
            //row 2
            //assumes 0.f
            //assumes 0.f
            //assumes 1.f
        };
        //HLSL (DirectX Shader Language)
        struct {
            //row 0
            ScnFLOAT _m00;
            ScnFLOAT _m01;
            ScnFLOAT _m02;
            //row 1
            ScnFLOAT _m10;
            ScnFLOAT _m11;
            ScnFLOAT _m12;
            //row 2
            //0.0f
            //0.0f
            //1.0f
        };
        ScnFLOAT r[3][2];  //for a 3x3 matrix, the last 3 elemments are allways asummed to be [0, 0, 1].
        ScnFLOAT e[6];     //for a 3x3 matrix, the last 3 elemments are allways asummed to be [0, 0, 1].
    };
} STScnMatrix2D;

//Transform

#ifndef SNC_COMPILING_SHADER
SC_INLN void ScnMatrix2D_translate(STScnMatrix2D* obj, const STScnPoint2D t){
    obj->e02 = (obj->e00 * t.x) + (obj->e01 * t.y) + obj->e02;
    obj->e12 = (obj->e10 * t.x) + (obj->e11 * t.y) + obj->e12;
}
#endif

#ifndef SNC_COMPILING_SHADER
SC_INLN void ScnMatrix2D_scale(STScnMatrix2D* obj, const STScnSize2D s){
    obj->e00 *= (s.width);
    obj->e10 *= (s.width);
    obj->e01 *= (s.height);
    obj->e11 *= (s.height);
}
#endif

#ifndef SNC_COMPILING_SHADER
SC_INLN void ScnMatrix2D_rotateRad(STScnMatrix2D* obj, const ScnFLOAT rad){
    const ScnFLOAT vSin    = ScnSinf(rad);
    const ScnFLOAT vCos    = ScnCosf(rad);
    const ScnFLOAT e00     = obj->e00;
    const ScnFLOAT e10     = obj->e10;
    obj->e00    = (e00 * vCos)  + (obj->e01 * vSin);
    obj->e01    = (e00 * -vSin) + (obj->e01 * vCos);
    obj->e10    = (e10 * vCos)  + (obj->e11 * vSin);
    obj->e11    = (e10 * -vSin) + (obj->e11 * vCos);
}
#endif

#ifndef SNC_COMPILING_SHADER
SC_INLN void ScnMatrix2D_rotateDeg(STScnMatrix2D* obj, const ScnFLOAT deg){
    const ScnFLOAT rad     = DEG_2_RAD(deg);
    const ScnFLOAT vSin    = ScnSinf(rad);
    const ScnFLOAT vCos    = ScnCosf(rad);
    const ScnFLOAT e00     = obj->e00;
    const ScnFLOAT e10     = obj->e10;
    obj->e00    = (e00 * vCos)  + (obj->e01 * vSin);
    obj->e01    = (e00 * -vSin) + (obj->e01 * vCos);
    obj->e10    = (e10 * vCos)  + (obj->e11 * vSin);
    obj->e11    = (e10 * -vSin) + (obj->e11 * vCos);
}
#endif

#ifndef SNC_COMPILING_SHADER
SC_INLN STScnMatrix2D ScnMatrix2D_multiply(const STScnMatrix2D* obj, const STScnMatrix2D* other){
    return (STScnMatrix2D){
        //row 0
        (obj->e00 * other->e00) + (obj->e01 * other->e10) /*allways zero: + (obj->e02 * other->e20)*/
        , (obj->e00 * other->e01) + (obj->e01 * other->e11) /*allways zero: + (obj->e02 * other->e21)*/
        , (obj->e00 * other->e02) + (obj->e01 * other->e12) + (obj->e02 /*siempre 1: * other->e22*/)
        //row 1
        , (obj->e10 * other->e00) + (obj->e11 * other->e10) /*allways cero: + (obj->e12 * other->e20)*/
        , (obj->e10 * other->e01) + (obj->e11 * other->e11) /*allways cero: + (obj->e12 * other->e21)*/
        , (obj->e10 * other->e02) + (obj->e11 * other->e12) + (obj->e12 /*allways 1: * other->e22*/)
    };
}
#endif

//Calculate

#ifndef SNC_COMPILING_SHADER
SC_INLN ScnFLOAT ScnMatrix2D_determinant(const STScnMatrix2D* obj){
    return ((obj->e00 * obj->e11) - (obj->e01 * obj->e10)); /*only 6 elems matrix*/
}
#endif

#ifndef SNC_COMPILING_SHADER
SC_INLN STScnMatrix2D ScnMatrix2D_inverse(const STScnMatrix2D* obj){
    const ScnFLOAT vDet = ((obj->e00 * obj->e11) - (obj->e01 * obj->e10));
    //NBASSERT(vDet != 0.0f && vDet != -0.0f) NBASSERT(vDet == vDet)
    if (vDet != 0.0f && vDet != -0.0f && vDet == vDet) {
        return (STScnMatrix2D){
            //row 0
            ((obj->e11 /** obj->e22*/) /*- (obj->e21 * obj->e12) always zero*/) / vDet
            , (/*(obj->e02 * obj->e21) always zero*/0.0f - (/*obj->e22 **/ obj->e01)) / vDet
            , ((obj->e01 * obj->e12) - (obj->e11 * obj->e02)) / vDet
            //row 1
            , (/*(obj->e12 * obj->e20) always zero*/0.0f - (/*obj->e22 **/ obj->e10)) / vDet
            , ((obj->e00 /** obj->e22*/) /*- (obj->e20 * obj->e02) always zero*/) / vDet
            , ((obj->e02 * obj->e10) - (obj->e12 * obj->e00)) / vDet
            //row 2
            //, ((obj->e10 * obj->e21) - (obj->e20 * obj->e11)) / vDet;
            //, ((obj->e01 * obj->e20) - (obj->e21 * obj->e00)) / vDet;
            //, ((obj->e00 * obj->e11) - (obj->e10 * obj->e01)) / vDet;
        };
    }
    return (STScnMatrix2D)STScnMatrix2D_Zero;
}
#endif

#ifndef SNC_COMPILING_SHADER
SC_INLN STScnMatrix2D ScnMatrix2D_fromTransforms(const STScnPoint2D traslation, const ScnFLOAT radRot, const STScnSize2D scale){
    const ScnFLOAT vSin = ScnSinf(radRot);
    const ScnFLOAT vCos = ScnCosf(radRot);
    return (STScnMatrix2D) {
        vCos * scale.width, -vSin * scale.height, traslation.x
        , vSin * scale.width, vCos * scale.height, traslation.y
    };
}
#endif

//Apply

#ifndef SNC_COMPILING_SHADER
SC_INLN STScnPoint2D ScnMatrix2D_applyToPoint(const STScnMatrix2D* obj, const STScnPoint2D p){
    return (STScnPoint2D) {
        (obj->e00 * p.x) + (obj->e01 * p.y) + obj->e02
        , (obj->e10 * p.x) + (obj->e11 * p.y) + obj->e12
    };
}
#endif

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnMatrix2D_h */
