//
//  ScnGpuFramebuff.h
//  ixtli-render
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnGpuFramebuff_h
#define ScnGpuFramebuff_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/gpu/ScnGpuFramebuffProps.h"

#ifdef __cplusplus
extern "C" {
#endif

//ENScnGpuFramebuffDstType

/** @enum ENScnGpuFramebuffDstType
 *  @brief Type of drawing destination for a framebuffer.
 */
typedef enum ENScnGpuFramebuffDstType {
    ENScnGpuFramebuffDstType_None = 0,
    ENScnGpuFramebuffDstType_Texture,
    ENScnGpuFramebuffDstType_Renderbuff,
    ENScnGpuFramebuffDstType_OSView,
    //Count
    ENScnGpuFramebuffDstType_Count
} ENScnGpuFramebuffDstType;

//STScnGpuFramebuffApiItf

/** @struct STScnGpuFramebuffApiItf
 *  @brief Frame buffer's API interface.
 *  @var STScnGpuFramebuffApiItf::free
 *  Method to free the frame buffer.
 *  @var STScnGpuFramebuffApiItf::getSize
 *  Method to retrieve the size of the framebuffer.
 *  @var STScnGpuFramebuffApiItf::syncSize
 *  Method to set the size of the frame buffer.
 *  @var STScnGpuFramebuffApiItf::getProps
 *  Method to get the properties of the frame buffer.
 *  @var STScnGpuFramebuffApiItf::setProps
 *  Method to set the properties of the frame buffer.
 */
typedef struct STScnGpuFramebuffApiItf {
    void            (*free)(void* data);
    //
    STScnSize2DU    (*getSize)(void* data);
    ScnBOOL         (*syncSize)(void* data, const STScnSize2DU size);
    //
    STScnGpuFramebuffProps (*getProps)(void* data);
    ScnBOOL         (*setProps)(void* data, const STScnGpuFramebuffProps* const props);
} STScnGpuFramebuffApiItf;


//ScnGpuFramebuffRef

/** @struct ScnGpuFramebuffRef
 *  @brief ScnGpuFramebuff shared pointer. An abstract object based on the currently used API.
 */

#define ScnGpuFramebuffRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnGpuFramebuff)

//

/**
 * @brief Prepares the gpu abstract object with the provided interface.
 * @param ref Reference to object.
 * @param itf Interface to the API.
 * @param itfParam Parameter to be given to the interface's  methods.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnGpuFramebuff_prepare(ScnGpuFramebuffRef ref, const STScnGpuFramebuffApiItf* itf, void* itfParam);

/**
 * @brief Retrieves the gpu abstract object pointer.
 * @param ref Reference to object.
 * @return Abstract object's pointer on success, NULL otherwise.
 */
void* ScnGpuFramebuff_getApiItfParam(ScnGpuFramebuffRef ref);

/**
 * @brief Retrieves the framebuffer's size.
 * @param ref Reference to object.
 * @return Size on success, STScnSize2DU_Zero otherwise.
 */
STScnSize2DU ScnGpuFramebuff_getSize(ScnGpuFramebuffRef ref);

/**
 * @brief Synchronizes the framebuffer's size.
 * @param ref Reference to object.
 * @param size The provided size. The object could ignore this parameter and read the real size from the framebuffer.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnGpuFramebuff_syncSize(ScnGpuFramebuffRef ref, const STScnSize2DU size);

/**
 * @brief Retrieves the framebuffer's properties.
 * @param ref Reference to object.
 * @return Properties on success, STScnGpuFramebuffProps_Zero otherwise.
 */
STScnGpuFramebuffProps  ScnGpuFramebuff_getProps(ScnGpuFramebuffRef ref);

/**
 * @brief Updates the framebuffer's properties.
 * @param ref Reference to object.
 * @param props The provided properties. The object could ignore this parameter and read the real properties from the framebuffer.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnGpuFramebuff_setProps(ScnGpuFramebuffRef ref, const STScnGpuFramebuffProps* const props);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuFramebuff_h */
