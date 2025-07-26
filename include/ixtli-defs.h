//
//  ixtli-defs.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ixtli_defs_h
#define ixtli_defs_h

#include <string.h> //for memset (in SCN_STRUCT_REF_METHODS_DEC)

#define SCN_DEBUG
//#define SCN_SILENT_MODE
//#define SCN_VERBOSE_MODE

#ifdef SCN_DEBUG
#   define SCN_ASSERTS_ACTIVATED
#endif

#ifdef SCN_ASSERTS_ACTIVATED
    #include <assert.h>         //assert
#   define SCN_ASSERT(EVAL)     { if(!(EVAL)){ SCN_PRINTF_ERROR("ASSERT, cond '"#EVAL"'.\n"); SCN_PRINTF_ERROR("ASSERT, file '%s'\n", __FILE__); SCN_PRINTF_ERROR("ASSERT, line %d.\n", __LINE__); assert(0); }}
#else
#   define SCN_ASSERT(EVAL)     ((void)0);
#endif

// PRINTING/LOG

#if defined(__ANDROID__) //Android
#   include <android/log.h>
#   define SCN_PRINTF_ALLWAYS(STR_FMT, ...)     __android_log_print(ANDROID_LOG_INFO, "Render", STR_FMT, ##__VA_ARGS__)
#elif defined(__QNX__) //BB10
#   include <stdio.h>
#   define SCN_PRINTF_ALLWAYS(STR_FMT, ...)     fprintf(stdout, "Render, " STR_FMT, ##__VA_ARGS__); fflush(stdout)
#else
#   include <stdio.h>
#   define SCN_PRINTF_ALLWAYS(STR_FMT, ...)     printf("Render, " STR_FMT, ##__VA_ARGS__)
#endif

#if defined(SCN_SILENT_MODE)
#   define SCN_PRINTF_INFO(STR_FMT, ...)        ((void)0)
#   define SCN_PRINTF_ERROR(STR_FMT, ...)       ((void)0)
#   define SCN_PRINTF_WARNING(STR_FMT, ...)     ((void)0)
#else
#   if defined(__ANDROID__) //Android
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

#define IX_INLN             static inline

#define SCN_FALSE           0
#define SCN_TRUE            1

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
typedef float               ScnFLOAT;   //float
typedef double              ScnDOUBLE;  //double

//Helpers

#define NBMemory_setZeroSt(ST, TYPE)    memset(&(ST), 0, sizeof(ST))

#define SCN_ITF_SET_MISSING_METHOD_TO_NOP(ITF, ITF_NAME, M_NAME) \
    if(itf->M_NAME == NULL) itf->M_NAME = ITF_NAME ## _nop_ ## M_NAME

#define SCN_ITF_SET_MISSING_METHOD_TO_DEFAULT(ITF, ITF_NAME, M_NAME) \
    if(itf->M_NAME == NULL) itf->M_NAME = ITF_NAME ## _default_ ## M_NAME

#define SCN_STRUCT_REF_METHODS_DEC(STNAME)  \
    ScnSI32                     STNAME ## _getOpqSz(void); \
    void                        STNAME ## _initZeroedOpq(STScnContextRef ctx, void* opq); \
    void                        STNAME ## _destroyOpq(void* opq); \
    \
    IX_INLN ScnBOOL             STNAME ## _isNull(ST ## STNAME ## Ref ref) { return (ref.ptr != NULL); } \
    IX_INLN void                STNAME ## _null(ST ## STNAME ## Ref* ref) { ref->ptr = NULL; } \
    IX_INLN void                STNAME ## _retain(ST ## STNAME ## Ref ref) { ScnSharedPtr_retain(ref.ptr); } \
    IX_INLN void                STNAME ## _release(ST ## STNAME ## Ref* ref) { if(0 == ScnSharedPtr_release(ref->ptr)) { ScnSharedPtr_free(ref->ptr); ref->ptr = NULL; } } \
    IX_INLN ST ## STNAME ## Ref STNAME ## _alloc(STScnContextRef ctx) \
    { \
        ST ## STNAME ## Ref r = { NULL }; \
        const ScnSI32 opqSz = STNAME ##_getOpqSz(); \
        void* opq = ScnContext_malloc(ctx, opqSz, #STNAME); \
        if(opq != NULL){ \
            STScnSharedPtr* ptr = ScnSharedPtr_alloc(ctx.itf, STNAME ##_destroyOpq, opq, #STNAME "_ptr"); \
            if(ptr != NULL){ \
                memset(opq, 0, opqSz); \
                STNAME ##_initZeroedOpq(ctx, opq); \
                r.ptr = ptr; opq = NULL; \
            } \
            if(opq != NULL){ \
                ScnContext_mfree(ctx, opq); \
            } \
        } \
        return r; \
    }
   
    
    

//tmp
#define NBMemory_alloc(SZ)              ((void*)0)
#define NBMemory_copy(DST, SCR, SZ)
#define NBMemory_free(PTR)              

#endif /* ixtli_defs_h */
