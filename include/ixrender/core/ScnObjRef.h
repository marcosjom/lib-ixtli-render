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

#define ScnObjRef_Zero    { NULL }

typedef struct ScnObjRef_ {
    STScnSharedPtr* ptr;
} ScnObjRef;

SC_INLN ScnBOOL ScnObjRef_isNull(ScnObjRef ref) { return (ref.ptr == NULL); }
SC_INLN ScnBOOL ScnObjRef_isSame(ScnObjRef ref, ScnObjRef other) { return (ref.ptr == other.ptr); }
SC_INLN void    ScnObjRef_null(ScnObjRef* ref) { ref->ptr = NULL; }
SC_INLN void    ScnObjRef_retain(ScnObjRef ref) { ScnSharedPtr_retain(ref.ptr); }
SC_INLN void    ScnObjRef_release(ScnObjRef* ref) { if(0 == ScnSharedPtr_release(ref->ptr)) { ScnSharedPtr_free(ref->ptr); ref->ptr = NULL; } }
SC_INLN void    ScnObjRef_releaseAndNullify(ScnObjRef* ref) { if(ref->ptr != NULL) { if(0 == ScnSharedPtr_release(ref->ptr)) { ScnSharedPtr_free(ref->ptr); ref->ptr = NULL; } ref->ptr = NULL; } }
SC_INLN void    ScnObjRef_set(ScnObjRef* ref, ScnObjRef other) { if(!ScnObjRef_isNull(other)){ ScnObjRef_retain(other); } if(!ScnObjRef_isNull(*ref)){ ScnObjRef_release(ref); } *ref = other; }

//

#define SCN_REF_STRUCT_METHODS_DEC(STNAME)  \
    typedef struct STNAME ## Ref_ { \
        STScnSharedPtr* ptr; \
    } STNAME ## Ref; \
    \
    ScnSI32                     STNAME ## _getOpqSz(void); \
    void                        STNAME ## _initZeroedOpq(ScnContextRef ctx, void* opq); \
    void                        STNAME ## _destroyOpq(void* opq); \
    \
    SC_INLN ScnBOOL             STNAME ## _isNull(STNAME ## Ref ref) { return (ref.ptr == NULL); } \
    SC_INLN ScnBOOL             STNAME ## _isSame(STNAME ## Ref ref, STNAME ## Ref other) { return (ref.ptr == other.ptr); } \
    SC_INLN void                STNAME ## _null(STNAME ## Ref* ref) { ref->ptr = NULL; } \
    SC_INLN void                STNAME ## _retain(STNAME ## Ref ref) { ScnSharedPtr_retain(ref.ptr); } \
    SC_INLN void                STNAME ## _release(STNAME ## Ref* ref) { if(0 == ScnSharedPtr_release(ref->ptr)) { ScnSharedPtr_free(ref->ptr); ref->ptr = NULL; } } \
    SC_INLN void                STNAME ## _releaseAndNullify(STNAME ## Ref* ref) { if(ref->ptr != NULL) { if(0 == ScnSharedPtr_release(ref->ptr)) { ScnSharedPtr_free(ref->ptr); ref->ptr = NULL; } ref->ptr = NULL; } } \
    SC_INLN void                STNAME ## _set(STNAME ## Ref* ref, STNAME ## Ref other) { if(!STNAME ## _isNull(other)){ STNAME ## _retain(other); } if(!STNAME ## _isNull(*ref)){ STNAME ## _release(ref); } *ref = other; } \
    SC_INLN STNAME ## Ref STNAME ## _alloc(ScnContextRef ctx) \
    { \
        STNAME ## Ref r = { NULL }; \
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
