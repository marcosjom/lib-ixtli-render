//
//  ScnBitmapColor.h
//  ixtli-render
//
//  Created by Marcos Ortega on 21/8/25.
//

#ifndef ScnBitmapColor_h
#define ScnBitmapColor_h

#ifdef __cplusplus
extern "C" {
#endif

/** @enum ENScnBitmapColor
 *  @brief Colors used by this library.
 *  @var ENScnBitmapColor undef
 *  Undefined color, this value is zero.
 *  @var ENScnBitmapColor ALPHA8
 *  Alpha only, RGB is assumed to white.
 *  @var ENScnBitmapColor GRAY8
 *  Gray only, RGB are the same value, alpha is assumed to opaque.
 *  @var ENScnBitmapColor GRAYALPHA8
 *  Gray and alpha, RGB are the same value.
 *  @var ENScnBitmapColor RGB8
 *  RGB only, alpha is assumed to opaque.
 *  @var ENScnBitmapColor RGBA8
 *  RGBA components.
 *  @var ENScnBitmapColor Count
 *  Enum's boundary.
 */
typedef enum ENScnBitmapColor {
    ENScnBitmapColor_undef = 0,
    //
    ENScnBitmapColor_ALPHA8,        //only alpha (8 bits)
    ENScnBitmapColor_GRAY8,         //grayscale (8 bits)
    ENScnBitmapColor_GRAYALPHA8,    //grayscale and alpha (8 bits each component)
    ENScnBitmapColor_RGB8,          //RGB (8 bits each component)
    ENScnBitmapColor_RGBA8,         //RGBA (8 bits each component)
    //
    ENScnBitmapColor_Count
} ENScnBitmapColor;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnBitmapColor_h */
