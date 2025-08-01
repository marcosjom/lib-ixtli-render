//
//  ScnColor.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnColor_h
#define ScnColor_h

#include "ixrender/ixtli-defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ScnColor_areEqual(ONE, OTHER)    ((ONE).r == (OTHER).r && (ONE).g == (OTHER).g && (ONE).b == (OTHER).b && (ONE).a == (OTHER).a)

#define STScnColor_Zero  { 0.f, 0.f, 0.f, 0.f }

typedef struct STScnColor {
    union {
        struct {
            ScnFLOAT r, g, b, a;
        };
        ScnFLOAT c[4];
    };
} STScnColor;

#define SCN_COLOR8_R_MSK             0xFF000000u    //1111-1111 0000-0000 0000-0000 0000-0000b
#define SCN_COLOR8_R_BIT_FIRST       0x1000000u    //0000-0001 0000-0000 0000-0000 0000-0000b
#define SCN_COLOR8_R_BIT_LAST        0x80000000u    //1000-0000 0000-0000 0000-0000 0000-0000b
#define SCN_COLOR8_R_MSK_MAX         0xFFu        //
//
#define SCN_COLOR8_G_MSK             0xFF0000u    //0000-0000 1111-1111 0000-0000 0000-0000b
#define SCN_COLOR8_G_BIT_FIRST       0x10000u    //0000-0000 0000-0001 0000-0000 0000-0000b
#define SCN_COLOR8_G_BIT_LAST        0x800000u    //0000-0000 1000-0000 0000-0000 0000-0000b
#define SCN_COLOR8_G_MSK_MAX         0xFFu        //
//
#define SCN_COLOR8_B_MSK             0xFFFF00u    //0000-0000 0000-0000 1111-1111 0000-0000b
#define SCN_COLOR8_B_BIT_FIRST       0x100u        //0000-0000 0000-0000 0000-0001 0000-0000b
#define SCN_COLOR8_B_BIT_LAST        0x8000u        //0000-0000 0000-0000 1000-0000 0000-0000b
#define SCN_COLOR8_B_MSK_MAX         0xFFu        //
//
#define SCN_COLOR8_A_MSK             0xFFu        //0000-0000 0000-0000 0000-0000 1111-1111b
#define SCN_COLOR8_A_BIT_FIRST       0x1u        //0000-0000 0000-0000 0000-0000 0000-0001b
#define SCN_COLOR8_A_BIT_LAST        0x80u        //0000-0000 0000-0000 0000-0000 1000-0000b
#define SCN_COLOR8_A_MSK_MAX         0xFFu        //

#define ScnColor8_getR(V32)          (((V32) / SCN_COLOR8_R_BIT_FIRST) % (SCN_COLOR8_R_MSK_MAX + 1u))
#define ScnColor8_getG(V32)          (((V32) / SCN_COLOR8_G_BIT_FIRST) % (SCN_COLOR8_G_MSK_MAX + 1u))
#define ScnColor8_getB(V32)          (((V32) / SCN_COLOR8_B_BIT_FIRST) % (SCN_COLOR8_B_MSK_MAX + 1u))
#define ScnColor8_getA(V32)          (((V32) / SCN_COLOR8_A_BIT_FIRST) % (SCN_COLOR8_A_MSK_MAX + 1u))

#define STScnColor8_Zero  { 0, 0, 0, 0 }
#define STScnColor8_255   { 255, 255, 255, 255 }

typedef struct STScnColor8 {
    union {
        struct {
            ScnUI8 r, g, b, a;
        };
        ScnUI8 c[4];
        ScnUI32 v32;
    };
} STScnColor8;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnColor_h */
