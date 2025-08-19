//
//  ScnObjRef.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#ifndef ScnObjRef_h
#define ScnObjRef_h

#include "ixrender/core/ScnContext.h"
#include "ixrender/core/ScnSharedPtr.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnObjRef

/** @struct ScnObjRef
 *  @brief Generic shared pointer referencing to an object.
 *  @var ScnObjRef::ptr
 *  Shared pointer
 */

#define ScnObjRef_Zero    { NULL }

typedef struct ScnObjRef {
    STScnSharedPtr* ptr;
} ScnObjRef;

/**
 * @brief Compares the references with NULL.
 * @param ref Reference to object.
 * @return ScnTRUE if references to NULL, ScnFALSE otherwise.
 */
SC_INLN ScnBOOL ScnObjRef_isNull(ScnObjRef ref) {
    return (ref.ptr == NULL);
}

/**
 * @brief Compares two references.
 * @param ref Reference to object.
 * @param other Reference to other object.
 * @return ScnTRUE if both references points to the same object (including NULL), ScnFALSE otherwise.
 */
SC_INLN ScnBOOL ScnObjRef_isSame(ScnObjRef ref, ScnObjRef other) {
    return (ref.ptr == other.ptr);
}

/**
 * @brief Nullifiies/detaches the reference without releasing it.
 * @param ref Reference to object.
 */
SC_INLN void ScnObjRef_null(ScnObjRef* ref) {
    ref->ptr = NULL;
}

/**
 * @brief Increases the reference's retain count.
 * @param ref Reference to object.
 */
SC_INLN void ScnObjRef_retain(ScnObjRef ref) {
    ScnSharedPtr_retain(ref.ptr);
}

/**
 * @brief Decreases the reference's retain count, and frees the object if the retain count reached zero.
 * @param ref Reference to object.
 */
SC_INLN void ScnObjRef_release(ScnObjRef* ref) {
    if (0 == ScnSharedPtr_release(ref->ptr)) {
        ScnSharedPtr_free(ref->ptr);
        ref->ptr = NULL;
    }
}

/**
 * @brief Decreases the reference's retain count and nullyfies the reference. Frees the object if the retain count reached zero.
 * @note This is the equivalent of calling 'release' and 'null'.
 * @param ref Reference to object.
 */
SC_INLN void ScnObjRef_releaseAndNull(ScnObjRef* ref) {
    if (ref->ptr != NULL) {
        if (0 == ScnSharedPtr_release(ref->ptr)) {
            ScnSharedPtr_free(ref->ptr);
        }
        ref->ptr = NULL;
    }
}

/**
 * @brief Sets the referenced object by retainning the new reference, and releasing the old reference afterwards.
 * @param ref Reference to object.
 * @param other Reference to new object.
 */
SC_INLN void ScnObjRef_set(ScnObjRef* ref, ScnObjRef other) {
    if (!ScnObjRef_isNull(other)) {
        ScnObjRef_retain(other);
    }
    if (!ScnObjRef_isNull(*ref)) {
        ScnObjRef_release(ref);
    }
    *ref = other;
}

/**
 * @brief Macro that defines the basic methods for a new referenceable object type.
 */

#define SCN_REF_STRUCT_METHODS_DEC(STNAME)  \
    typedef struct STNAME ## Ref { \
        STScnSharedPtr* ptr; \
    } STNAME ## Ref; \
    \
    ScnUI32                     STNAME ## _getOpqSz(void); \
    void                        STNAME ## _initZeroedOpq(ScnContextRef ctx, void* opq); \
    void                        STNAME ## _destroyOpq(void* opq); \
    \
    SC_INLN ScnBOOL             STNAME ## _isNull(STNAME ## Ref ref) { return (ref.ptr == NULL); } \
    SC_INLN ScnBOOL             STNAME ## _isSame(STNAME ## Ref ref, STNAME ## Ref other) { return (ref.ptr == other.ptr); } \
    SC_INLN void                STNAME ## _null(STNAME ## Ref* ref) { ref->ptr = NULL; } \
    SC_INLN void                STNAME ## _retain(STNAME ## Ref ref) { ScnSharedPtr_retain(ref.ptr); } \
    SC_INLN void                STNAME ## _release(STNAME ## Ref* ref) { if(0 == ScnSharedPtr_release(ref->ptr)) { ScnSharedPtr_free(ref->ptr); ref->ptr = NULL; } } \
    SC_INLN void                STNAME ## _releaseAndNull(STNAME ## Ref* ref) { if(ref->ptr != NULL) { if(0 == ScnSharedPtr_release(ref->ptr)) { ScnSharedPtr_free(ref->ptr); ref->ptr = NULL; } ref->ptr = NULL; } } \
    SC_INLN void                STNAME ## _set(STNAME ## Ref* ref, STNAME ## Ref other) { if(!STNAME ## _isNull(other)){ STNAME ## _retain(other); } if(!STNAME ## _isNull(*ref)){ STNAME ## _release(ref); } *ref = other; } \
    SC_INLN STNAME ## Ref STNAME ## _alloc(ScnContextRef ctx) \
    { \
        STNAME ## Ref r = { NULL }; \
        const ScnUI32 opqSz = STNAME ##_getOpqSz(); \
        void* opq = ScnContext_malloc(ctx, opqSz, #STNAME); \
        if(opq != NULL){ \
            STScnSharedPtr* ptr = ScnSharedPtr_alloc(ctx.itf, STNAME ##_destroyOpq, opq, #STNAME "_ptr"); \
            if(ptr != NULL){ \
                ScnMemset(opq, 0, opqSz); \
                STNAME ##_initZeroedOpq(ctx, opq); \
                r.ptr = ptr; opq = NULL; \
            } \
            if(opq != NULL){ \
                ScnContext_mfree(ctx, opq); \
            } \
        } \
        return r; \
    }

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnObjRef_h */
