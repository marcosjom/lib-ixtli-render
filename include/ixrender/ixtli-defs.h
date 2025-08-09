//
//  ixtli-defs.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ixtli_defs_h
#define ixtli_defs_h

#define SCN_DEBUG
//#define SCN_SILENT_MODE
#define SCN_VERBOSE_MODE

#ifdef SCN_DEBUG
#   define SCN_ASSERTS_ACTIVATED
#endif

#if defined(__METAL__) || defined(__METAL_MACOS__) || defined(__METAL_IOS__)
#   define SNC_COMPILING_SHADER
#endif

#ifdef SCN_ASSERTS_ACTIVATED
#   if defined(__METAL__) || defined(__METAL_MACOS__) || defined(__METAL_IOS__)
#       define SCN_ASSERT(EVAL) ((void)0);
#   else
#   include <assert.h> //assert()
#       define SCN_ASSERT(EVAL) { if(!(EVAL)){ SCN_PRINTF_ERROR("ASSERT failed.\n"); SCN_PRINTF_ERROR("ASSERT, file '%s'\n", __FILE__); SCN_PRINTF_ERROR("ASSERT, line %d.\n", __LINE__); assert(0); }}
#   endif
#else
#   define SCN_ASSERT(EVAL)     ((void)0);
#endif

// PRINTING/LOG

#if defined(SNC_COMPILING_SHADER)
#   define SCN_PRINTF_ALLWAYS(STR_FMT, ...)     ((void)0)
#elif defined(__ANDROID__)
#   include <android/log.h> //__android_log_print()
#   define SCN_PRINTF_ALLWAYS(STR_FMT, ...)     __android_log_print(ANDROID_LOG_INFO, "Render", STR_FMT, ##__VA_ARGS__)
#elif defined(__QNX__) //BB10
#   include <stdio.h> //fprintf()
#   define SCN_PRINTF_ALLWAYS(STR_FMT, ...)     fprintf(stdout, "Render, " STR_FMT, ##__VA_ARGS__); fflush(stdout)
#else
#   include <stdio.h> //printf()
#   define SCN_PRINTF_ALLWAYS(STR_FMT, ...)     printf("Render, " STR_FMT, ##__VA_ARGS__)
#endif

#if defined(SCN_SILENT_MODE)
#   define SCN_PRINTF_INFO(STR_FMT, ...)        ((void)0)
#   define SCN_PRINTF_ERROR(STR_FMT, ...)       ((void)0)
#   define SCN_PRINTF_WARNING(STR_FMT, ...)     ((void)0)
#else
#   if defined(SNC_COMPILING_SHADER)
#       define SCN_PRINTF_INFO(STR_FMT, ...)    ((void)0)
#       define SCN_PRINTF_ERROR(STR_FMT, ...)   ((void)0)
#       define SCN_PRINTF_WARNING(STR_FMT, ...) ((void)0)
#   elif defined(__ANDROID__)
#        ifndef SCN_VERBOSE_MODE
#        define SCN_PRINTF_INFO(STR_FMT, ...)   ((void)0)
#        else
#        define SCN_PRINTF_INFO(STR_FMT, ...)   __android_log_print(ANDROID_LOG_INFO, "Render", STR_FMT, ##__VA_ARGS__)
#        endif
#        define SCN_PRINTF_ERROR(STR_FMT, ...)  __android_log_print(ANDROID_LOG_ERROR, "Render", "ERROR, "STR_FMT, ##__VA_ARGS__)
#        define SCN_PRINTF_WARNING(STR_FMT, ...) __android_log_print(ANDROID_LOG_WARN, "Render", "WARNING, "STR_FMT, ##__VA_ARGS__)
#   elif defined(__QNX__) //BB10
#        ifndef SCN_VERBOSE_MODE
#        define SCN_PRINTF_INFO(STR_FMT, ...)   ((void)0)
#        else
#        define SCN_PRINTF_INFO(STR_FMT, ...)   fprintf(stdout, "Render, " STR_FMT, ##__VA_ARGS__); fflush(stdout)
#        endif
#        define SCN_PRINTF_ERROR(STR_FMT, ...)  fprintf(stderr, "Render ERROR, " STR_FMT, ##__VA_ARGS__); fflush(stderr)
#        define SCN_PRINTF_WARNING(STR_FMT, ...) fprintf(stdout, "Render WARNING, " STR_FMT, ##__VA_ARGS__); fflush(stdout)
#   else
#        ifndef SCN_VERBOSE_MODE
#        define SCN_PRINTF_INFO(STR_FMT, ...)   ((void)0)
#        else
#        define SCN_PRINTF_INFO(STR_FMT, ...)   printf("Render, " STR_FMT, ##__VA_ARGS__)
#        endif
#       define SCN_PRINTF_ERROR(STR_FMT, ...)   printf("Render ERROR, " STR_FMT, ##__VA_ARGS__)
#       define SCN_PRINTF_WARNING(STR_FMT, ...) printf("Render WARNING, " STR_FMT, ##__VA_ARGS__)
#   endif
#endif

#define SC_INLN     static inline

//Helpers

#define ScnMemory_setZeroSt(ST)     ScnMemset(&(ST), 0, sizeof(ST))

#define SCN_ITF_SET_MISSING_METHOD_TO_NOP(ITF, ITF_NAME, M_NAME) \
    if(itf->M_NAME == NULL) itf->M_NAME = ITF_NAME ## _nop_ ## M_NAME

#define SCN_ITF_SET_MISSING_METHOD_TO_DEFAULT(ITF, ITF_NAME, M_NAME) \
    if(itf->M_NAME == NULL) itf->M_NAME = ITF_NAME ## _default_ ## M_NAME

//tmp
#define ScnMemory_alloc(SZ)              ((void*)0)
#define ScnMemory_copy(DST, SCR, SZ)
#define ScnMemory_free(PTR)              

//PI and PIx2
#ifndef PI
#    define PI              3.14159265359f //3.14159265f
#endif
#ifndef PIx2
#    define PIx2            6.28318530718f //6.2831853f
#endif

#ifndef RAD_2_DEG
#   define RAD_2_DEG(RAD)   ((RAD) * 180.f / PI)
#endif

#ifndef DEG_2_RAD
#   define DEG_2_RAD(DEG)   ((DEG) * PI / 180.f)
#endif

//NULL
#if !defined(__cplusplus) && !defined(NULL)
#    define NULL            ((void*)0)
#endif

#define ScnFALSE            0
#define ScnTRUE             1

// Data types

typedef unsigned char       ScnBOOL;    //ScnBOOL, Unsigned 8-bit integer value
typedef unsigned char       ScnBYTE;    //ScnBYTE, Unsigned 8-bit integer value
typedef char                ScnSI8;     //NIXSI8, Signed 8-bit integer value
typedef short int           ScnSI16;    //NIXSI16, Signed 16-bit integer value
typedef int                 ScnSI32;    //NIXSI32, Signed 32-bit integer value
typedef unsigned char       ScnUI8;     //ScnUI8, Unsigned 8-bit integer value
typedef unsigned short int  ScnUI16;    //ScnUI16, Unsigned 16-bit integer value
typedef unsigned int        ScnUI32;    //ScnUI32, Unsigned 32-bit integer value
typedef float               ScnFLOAT;   //ScnFLOAT
#if !defined(SNC_COMPILING_SHADER)
typedef long long           ScnSI64;    //ScnSI64, Signed 64-bit integer value
typedef unsigned long long  ScnUI64;    //ScnUI64[n], Unsigned 64-bit array—n is the number of array elements
typedef double              ScnDOUBLE;  //double
#endif

#if !defined(SNC_COMPILING_SHADER)

#ifdef __cplusplus
extern "C" {
#endif

//string.h
void* ScnMemset(void* dst, const ScnSI32 ch, const ScnUI32 count);
void* ScnMemcpy(void* dst, const void* src, const ScnUI32 count);
void* ScnMemmove(void* dst, const void* src, const ScnUI32 count);

//math.h
ScnFLOAT ScnSinf(ScnFLOAT num);
ScnFLOAT ScnCosf(ScnFLOAT num);

#ifdef __cplusplus
} //extern "C"
#endif

#endif //!defined(SNC_COMPILING_SHADER)

#endif /* ixtli_defs_h */
