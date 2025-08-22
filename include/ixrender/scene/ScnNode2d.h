//
//  ScnNode2d.h
//  ixtli-render
//
//  Created by Marcos Ortega on 02/8/25.
//

#ifndef ScnNode2d_h
#define ScnNode2d_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/core/ScnObjRef.h"
#include "ixrender/type/ScnColor.h"
#include "ixrender/type/ScnPoint.h"
#include "ixrender/type/ScnSize.h"
#include "ixrender/scene/ScnTransform2d.h"
#include "ixrender/scene/ScnNode2dProps.h"

#ifdef __cplusplus
extern "C" {
#endif

//ScnNode2dRef

/** @struct ScnNode2dRef
 *  @brief ScnNode2d shared pointer. An object that contains the scene relative properties (color and matrix multipliers) for a render node.
 */

#define ScnNode2dRef_Zero   ScnObjRef_Zero

SCN_REF_STRUCT_METHODS_DEC(ScnNode2d)

//props

/**
 * @brief Retrieves the properties of the scene node.
 * @param ref Reference to object.
 * @return The properties on success, STScnNode2dProps_Zero otherwise.
 */
STScnNode2dProps ScnNode2d_getProps(ScnNode2dRef ref);

//color

/**
 * @brief Retrieves the color multiplier of the scene node.
 * @param ref Reference to object.
 * @return The color on success, STScnColor8_Zero otherwise.
 */
STScnColor8 ScnNode2d_getColor8(ScnNode2dRef ref);

/**
 * @brief Sets the color multiplier of the scene node.
 * @param ref Reference to object.
 * @param color Color multiplier.
 */
void ScnNode2d_setColor8(ScnNode2dRef ref, const STScnColor8 color);

/**
 * @brief Sets the color multiplier of the scene node.
 * @param ref Reference to object.
 * @param r Red component color multiplier.
 * @param g Green component color multiplier.
 * @param b Blue component color multiplier.
 * @param a Alpha component color multiplier.
 */
void ScnNode2d_setColorRGBA8(ScnNode2dRef ref, const ScnUI8 r, const ScnUI8 g, const ScnUI8 b, const ScnUI8 a);

//transform

/**
 * @brief Retrieves the transformations of the scene node.
 * @param ref Reference to object.
 * @return The transformation on success, STScnTransform2d_Zero otherwise.
 */
STScnTransform2d ScnNode2d_getTransform(ScnNode2dRef ref);

/**
 * @brief Retrieves the translation of the scene node.
 * @param ref Reference to object.
 * @return The translation on success, STScnPoint2D_Zero otherwise.
 */
STScnPoint2D ScnNode2d_getTranslate(ScnNode2dRef ref);

/**
 * @brief Retrieves the scale of the scene node.
 * @param ref Reference to object.
 * @return The scale on success, STScnSize2D_Zero otherwise.
 */
STScnSize2D ScnNode2d_getScale(ScnNode2dRef ref);

/**
 * @brief Retrieves the rotation in degrees of the scene node.
 * @param ref Reference to object.
 * @return The rotation degrees on success, zero otherwise.
 */
ScnFLOAT ScnNode2d_getRotDeg(ScnNode2dRef ref);

/**
 * @brief Retrieves the rotation in radians of the scene node.
 * @param ref Reference to object.
 * @return The rotation radians on success, zero otherwise.
 */
ScnFLOAT ScnNode2d_getRotRad(ScnNode2dRef ref);

/**
 * @brief Sets the translation of the scene node.
 * @param ref Reference to object.
 * @param pos Translation to be set.
 */
void ScnNode2d_setTranslate(ScnNode2dRef ref, const STScnPoint2D pos);

/**
 * @brief Sets the translation of the scene node.
 * @param ref Reference to object.
 * @param x X translation to be set.
 * @param y Y translation to be set.
 */
void ScnNode2d_setTranslateXY(ScnNode2dRef ref, const ScnFLOAT x, const ScnFLOAT y);

/**
 * @brief Sets the scale of the scene node.
 * @param ref Reference to object.
 * @param s Scale to be set.
 */
void ScnNode2d_setScale(ScnNode2dRef ref, const STScnSize2D s);

/**
 * @brief Sets the scale of the scene node.
 * @param ref Reference to object.
 * @param sw Width scale to be set.
 * @param sh Height scale to be set.
 */
void ScnNode2d_setScaleWH(ScnNode2dRef ref, const ScnFLOAT sw, const ScnFLOAT sh);

/**
 * @brief Sets the rotation in degrees of the scene node.
 * @param ref Reference to object.
 * @param deg Rotation in degrees.
 */
void ScnNode2d_setRotDeg(ScnNode2dRef ref, const ScnFLOAT deg);

/**
 * @brief Sets the rotation in radians of the scene node.
 * @param ref Reference to object.
 * @param rad Rotation in radians.
 */
void ScnNode2d_setRotRad(ScnNode2dRef ref, const ScnFLOAT rad);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnNode2d_h */
