//
//  ScnVertexType.h
//  ixtli-render
//
//  Created by Marcos Ortega on 21/8/25.
//

#ifndef ScnVertexType_h
#define ScnVertexType_h

#ifdef __cplusplus
extern "C" {
#endif

//ENScnVertexType

/** @enum ENScnVertexType
 *  @brief Predefined vertices types (zero, one, two or three textures coordinates).
 */

#define ENScnVertexType_Count_Zeroes    { 0, 0, 0, 0 }

typedef enum ENScnVertexType {
    ENScnVertexType_2DColor = 0,    //2D, no texture
    ENScnVertexType_2DTex,          //2D, one texture
    ENScnVertexType_2DTex2,         //2D, two textures
    ENScnVertexType_2DTex3,         //2D, three textures
    //
    ENScnVertexType_Count
} ENScnVertexType;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnVertexType_h */
