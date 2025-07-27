//
//  ixtli-defs.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ixtli_defs_h
#define ixtli_defs_h

#define Scn_DEBUG
//#define Scn_SILENT_MODE
//#define Scn_VERBOSE_MODE

#ifdef Scn_DEBUG
#   define SCN_ASSERTS_ACTIVATED
#endif

#ifdef SCN_ASSERTS_ACTIVATED
    #include <assert.h>         //assert
#   define SCN_ASSERT(EVAL)     { if(!(EVAL)){ Scn_PRINTF_ERROR("ASSERT, cond '"#EVAL"'.\n"); Scn_PRINTF_ERROR("ASSERT, file '%s'\n", __FILE__); Scn_PRINTF_ERROR("ASSERT, line %d.\n", __LINE__); assert(0); }}
#else
#   define SCN_ASSERT(EVAL)     ((void)0);
#endif

// PRINTING/LOG

#if defined(__ANDROID__) //Android
#   include <android/log.h>
#   define Scn_PRINTF_ALLWAYS(STR_FMT, ...)     __android_log_print(ANDROID_LOG_INFO, "Render", STR_FMT, ##__VA_ARGS__)
#elif defined(__QNX__) //BB10
#   include <stdio.h>
#   define Scn_PRINTF_ALLWAYS(STR_FMT, ...)     fprintf(stdout, "Render, " STR_FMT, ##__VA_ARGS__); fflush(stdout)
#else
#   include <stdio.h>
#   define Scn_PRINTF_ALLWAYS(STR_FMT, ...)     printf("Render, " STR_FMT, ##__VA_ARGS__)
#endif

#if defined(Scn_SILENT_MODE)
#   define Scn_PRINTF_INFO(STR_FMT, ...)        ((void)0)
#   define Scn_PRINTF_ERROR(STR_FMT, ...)       ((void)0)
#   define Scn_PRINTF_WARNING(STR_FMT, ...)     ((void)0)
#else
#   if defined(__ANDROID__) //Android
#        ifndef Scn_VERBOSE_MODE
#        define Scn_PRINTF_INFO(STR_FMT, ...)   ((void)0)
#        else
#        define Scn_PRINTF_INFO(STR_FMT, ...)   __android_log_print(ANDROID_LOG_INFO, "Render", STR_FMT, ##__VA_ARGS__)
#        endif
#        define Scn_PRINTF_ERROR(STR_FMT, ...)  __android_log_print(ANDROID_LOG_ERROR, "Render", "ERROR, "STR_FMT, ##__VA_ARGS__)
#        define Scn_PRINTF_WARNING(STR_FMT, ...) __android_log_print(ANDROID_LOG_WARN, "Render", "WARNING, "STR_FMT, ##__VA_ARGS__)
#   elif defined(__QNX__) //BB10
#        ifndef Scn_VERBOSE_MODE
#        define Scn_PRINTF_INFO(STR_FMT, ...)   ((void)0)
#        else
#        define Scn_PRINTF_INFO(STR_FMT, ...)   fprintf(stdout, "Render, " STR_FMT, ##__VA_ARGS__); fflush(stdout)
#        endif
#        define Scn_PRINTF_ERROR(STR_FMT, ...)  fprintf(stderr, "Render ERROR, " STR_FMT, ##__VA_ARGS__); fflush(stderr)
#        define Scn_PRINTF_WARNING(STR_FMT, ...) fprintf(stdout, "Render WARNING, " STR_FMT, ##__VA_ARGS__); fflush(stdout)
#   else
#        ifndef Scn_VERBOSE_MODE
#        define Scn_PRINTF_INFO(STR_FMT, ...)   ((void)0)
#        else
#        define Scn_PRINTF_INFO(STR_FMT, ...)   printf("Render, " STR_FMT, ##__VA_ARGS__)
#        endif
#       define Scn_PRINTF_ERROR(STR_FMT, ...)   printf("Render ERROR, " STR_FMT, ##__VA_ARGS__)
#       define Scn_PRINTF_WARNING(STR_FMT, ...) printf("Render WARNING, " STR_FMT, ##__VA_ARGS__)
#   endif
#endif

#define SC_INLN             static inline

#define Scn_FALSE           0
#define Scn_TRUE            1

//NULL
#if !defined(__cplusplus) && !defined(NULL)
#    define NULL            ((void*)0)
#endif

// Data types

typedef unsigned char       ScnBOOL;    //ScnBOOL, Unsigned 8-bit integer value
typedef unsigned char       ScnBYTE;    //ScnBYTE, Unsigned 8-bit integer value
typedef char                ScnSI8;     //NIXSI8, Signed 8-bit integer value
typedef short int           ScnSI16;    //NIXSI16, Signed 16-bit integer value
typedef int                 ScnSI32;    //NIXSI32, Signed 32-bit integer value
typedef long long           ScnSI64;    //ScnSI64, Signed 64-bit integer value
typedef unsigned char       ScnUI8;     //ScnUI8, Unsigned 8-bit integer value
typedef unsigned short int  ScnUI16;    //ScnUI16, Unsigned 16-bit integer value
typedef unsigned int        ScnUI32;    //ScnUI32, Unsigned 32-bit integer value
typedef unsigned long long  ScnUI64;    //ScnUI64[n], Unsigned 64-bit array—n is the number of array elements
typedef float               ScnFLOAT;   //ScnFLOAT
typedef double              ScnDOUBLE;  //double

//Helpers

#define ScnMemory_setZeroSt(ST, TYPE)    memset(&(ST), 0, sizeof(ST))

#define SCN_ITF_SET_MISSING_METHOD_TO_NOP(ITF, ITF_NAME, M_NAME) \
    if(itf->M_NAME == NULL) itf->M_NAME = ITF_NAME ## _nop_ ## M_NAME

#define SCN_ITF_SET_MISSING_METHOD_TO_DEFAULT(ITF, ITF_NAME, M_NAME) \
    if(itf->M_NAME == NULL) itf->M_NAME = ITF_NAME ## _default_ ## M_NAME

//tmp
#define ScnMemory_alloc(SZ)              ((void*)0)
#define ScnMemory_copy(DST, SCR, SZ)
#define ScnMemory_free(PTR)              

#endif /* ixtli_defs_h */
