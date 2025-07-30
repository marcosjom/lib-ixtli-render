//
//  ScnObjRef.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnObjRef_h
#define ScnObjRef_h

#include <string.h> //for memset (in SCN_REF_STRUCT_METHODS_DEC)
#include "ixrender/core/ScnContext.h"
#include "ixrender/core/ScnSharedPtr.h"

#define STScnObjRef_Zero    { NULL }

typedef struct STScnObjRef_ {
    STScnSharedPtr* ptr;
} STScnObjRef;

SC_INLN ScnBOOL ScnObjRef_isNull(STScnObjRef ref) { return (ref.ptr == NULL); }
SC_INLN ScnBOOL ScnObjRef_isSame(STScnObjRef ref, STScnObjRef other) { return (ref.ptr == other.ptr); }
SC_INLN void    ScnObjRef_null(STScnObjRef* ref) { ref->ptr = NULL; }
SC_INLN void    ScnObjRef_retain(STScnObjRef ref) { ScnSharedPtr_retain(ref.ptr); }
SC_INLN void    ScnObjRef_release(STScnObjRef* ref) { if(0 == ScnSharedPtr_release(ref->ptr)) { ScnSharedPtr_free(ref->ptr); ref->ptr = NULL; } }
SC_INLN void    ScnObjRef_set(STScnObjRef* ref, STScnObjRef other) { if(!ScnObjRef_isNull(other)){ ScnObjRef_retain(other); } if(!ScnObjRef_isNull(*ref)){ ScnObjRef_release(ref); } *ref = other; }

//

#define SCN_REF_STRUCT_METHODS_DEC(STNAME)  \
    typedef struct ST ## STNAME ## Ref_ { \
        STScnSharedPtr* ptr; \
    } ST ## STNAME ## Ref; \
    \
    ScnSI32                     STNAME ## _getOpqSz(void); \
    void                        STNAME ## _initZeroedOpq(STScnContextRef ctx, void* opq); \
    void                        STNAME ## _destroyOpq(void* opq); \
    \
    SC_INLN ScnBOOL             STNAME ## _isNull(ST ## STNAME ## Ref ref) { return (ref.ptr == NULL); } \
    SC_INLN ScnBOOL             STNAME ## _isSame(ST ## STNAME ## Ref ref, ST ## STNAME ## Ref other) { return (ref.ptr == other.ptr); } \
    SC_INLN void                STNAME ## _null(ST ## STNAME ## Ref* ref) { ref->ptr = NULL; } \
    SC_INLN void                STNAME ## _retain(ST ## STNAME ## Ref ref) { ScnSharedPtr_retain(ref.ptr); } \
    SC_INLN void                STNAME ## _release(ST ## STNAME ## Ref* ref) { if(0 == ScnSharedPtr_release(ref->ptr)) { ScnSharedPtr_free(ref->ptr); ref->ptr = NULL; } } \
    SC_INLN void                STNAME ## _set(ST ## STNAME ## Ref* ref, ST ## STNAME ## Ref other) { if(!STNAME ## _isNull(other)){ STNAME ## _retain(other); } if(!STNAME ## _isNull(*ref)){ STNAME ## _release(ref); } *ref = other; } \
    SC_INLN ST ## STNAME ## Ref STNAME ## _alloc(STScnContextRef ctx) \
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

#endif /* ScnObjRef_h */
