//
//  ScnGpuRenderbuff.h
//  ixtli-render-mac
//
//  Created by Marcos Ortega on 26/7/25.
//

#ifndef ScnGpuRenderbuff_h
#define ScnGpuRenderbuff_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/type/ScnBitmap.h"

#ifdef __cplusplus
extern "C" {
#endif

//STScnGpuRenderbuffCfg

/** @struct STScnGpuRenderbuffCfg
 *  @brief Render buffer's configuration.
 *  @var STScnGpuRenderbuffCfg::color
 *  Render buffer's color.
 *  @var STScnGpuRenderbuffApiItf::width
 *  Render buffer's width.
 *  @var STScnGpuRenderbuffApiItf::height
 *  Render buffer's height.
 */

#define STScnGpuRenderbuffCfg_Zero   { ENScnBitmapColor_undef, 0, 0 }

typedef struct STScnGpuRenderbuffCfg {
    ENScnBitmapColor color;
    ScnUI32         width;
    ScnUI32         height;
} STScnGpuRenderbuffCfg;

//STScnGpuRenderbuffChanges

/** @struct STScnGpuRenderbuffChanges
 *  @brief Render buffer's changes.
 *  @var STScnGpuRenderbuffCfg::dummy
 *  Pending to impement.
 */

typedef struct STScnGpuRenderbuffChanges {
    ScnUI32 dummy;  //nothing
} STScnGpuRenderbuffChanges;

//STScnGpuRenderbuffApiItf

/** @struct STScnGpuRenderbuffApiItf
 *  @brief Render buffer's API interface.
 *  @var STScnGpuRenderbuffApiItf::free
 *  Method to free the render buffer.
 *  @var STScnGpuRenderbuffApiItf::sync
 *  Method to synchronize the renderbuffer.
 */
typedef struct STScnGpuRenderbuffApiItf {
    void    (*free)(void* data, void* usrData);
    //
    ScnBOOL (*sync)(void* data, const STScnGpuRenderbuffCfg* cfg, const STScnGpuRenderbuffChanges* changes, void* usrData);
} STScnGpuRenderbuffApiItf;

//ScnGpuRenderbuffRef

/** @struct ScnGpuRenderbuffRef
 *  @brief ScnGpuRenderbuff shared pointer. An abstract object based on the currently used API.
 */

#define ScnGpuRenderbuffRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnGpuRenderbuff)

//

/**
 * @brief Prepares the gpu abstract object with the provided interface.
 * @param ref Reference to object.
 * @param cfg Object's configuration.
 * @param itf Interface to the API.
 * @param itfParam Parameter to be given to the interface's  methods.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnGpuRenderbuff_prepare(ScnGpuRenderbuffRef ref, const STScnGpuRenderbuffCfg* cfg, const STScnGpuRenderbuffApiItf* itf, void* itfParam);


#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnGpuRenderbuff_h */
