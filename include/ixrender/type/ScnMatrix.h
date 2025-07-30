//
//  ScnMatrix.h
//  ixtli-render
//
//  Created by Marcos Ortega on 30/7/25.
//

#ifndef ScnMatrix_h
#define ScnMatrix_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/type/ScnPoint.h"
#include "ixrender/type/ScnSize.h"

#ifdef __OBJC__
#   import <Foundation/Foundation.h> //for sqrt(), sin(), cos(), etc...
#else
#   include <math.h> //for sqrt(), sin(), cos(), etc...
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define STScnMatrix_Zero        { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f }
#define STScnMatrix_Identity    { 1.f, 0.f, 0.f, 0.f, 1.f, 0.f }

typedef struct STScnMatrix_ {
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
            float _m00;
            float _m01;
            float _m02;
            //row 1
            float _m10;
            float _m11;
            float _m12;
            //row 2
            //0.0f
            //0.0f
            //1.0f
        };
        float r[3][2];  //for a 3x3 matrix, the last 3 elemments are allways asummed to be [0, 0, 1].
        float e[6];     //for a 3x3 matrix, the last 3 elemments are allways asummed to be [0, 0, 1].
    };
    ScnFLOAT    x;
    ScnFLOAT    y;
    ScnFLOAT    width;
    ScnFLOAT    height;
} STScnMatrix;

//Transform

SC_INLN void ScnMatrix_translate(STScnMatrix* obj, const STScnPoint t){
    obj->e02 = (obj->e00 * t.x) + (obj->e01 * t.y) + obj->e02;
    obj->e12 = (obj->e10 * t.x) + (obj->e11 * t.y) + obj->e12;
}

SC_INLN void ScnMatrix_scale(STScnMatrix* obj, const STScnSize s){
    obj->e00 *= (s.width);
    obj->e10 *= (s.width);
    obj->e01 *= (s.height);
    obj->e11 *= (s.height);
}

SC_INLN void ScnMatrix_rotateRad(STScnMatrix* obj, const float rad){
    const float vSin    = sin(rad);
    const float vCos    = cos(rad);
    const float e00     = obj->e00;
    const float e10     = obj->e10;
    obj->e00    = (e00 * vCos)  + (obj->e01 * vSin);
    obj->e01    = (e00 * -vSin) + (obj->e01 * vCos);
    obj->e10    = (e10 * vCos)  + (obj->e11 * vSin);
    obj->e11    = (e10 * -vSin) + (obj->e11 * vCos);
}

SC_INLN void ScnMatrix_rotateDeg(STScnMatrix* obj, const float deg){
    const float rad     = DEG_2_RAD(deg);
    const float vSin    = sin(rad);
    const float vCos    = cos(rad);
    const float e00     = obj->e00;
    const float e10     = obj->e10;
    obj->e00    = (e00 * vCos)  + (obj->e01 * vSin);
    obj->e01    = (e00 * -vSin) + (obj->e01 * vCos);
    obj->e10    = (e10 * vCos)  + (obj->e11 * vSin);
    obj->e11    = (e10 * -vSin) + (obj->e11 * vCos);
}

SC_INLN STScnMatrix ScnMatrix_multiply(const STScnMatrix* obj, const STScnMatrix* other){
    return (STScnMatrix){
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

//Calculate

SC_INLN float ScnMatrix_determinant(const STScnMatrix* obj){
    return ((obj->e00 * obj->e11) - (obj->e01 * obj->e10)); /*only 6 elems matrix*/
}

SC_INLN STScnMatrix ScnMatrix_inverse(const STScnMatrix* obj){
    const float vDet = ((obj->e00 * obj->e11) - (obj->e01 * obj->e10));
    //NBASSERT(vDet != 0.0f && vDet != -0.0f) NBASSERT(vDet == vDet)
    if (vDet != 0.0f && vDet != -0.0f && vDet == vDet) {
        return (STScnMatrix){
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
    return (STScnMatrix)STScnMatrix_Zero;
}

SC_INLN STScnMatrix ScnMatrix_fromTransforms(const STScnPoint traslation, const float radRot, const STScnSize scale){
    const float vSin = sin(radRot);
    const float vCos = cos(radRot);
    return (STScnMatrix) {
        vCos * scale.width, -vSin * scale.height, traslation.x
        , vSin * scale.width, vCos * scale.height, traslation.y
    };
}

//Apply

SC_INLN STScnPoint ScnMatrix_applyToPoint(const STScnMatrix* obj, const STScnPoint p){
    return (STScnPoint) {
        (obj->e00 * p.x) + (obj->e01 * p.y) + obj->e02
        , (obj->e10 * p.x) + (obj->e11 * p.y) + obj->e12
    };
}

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnMatrix_h */
