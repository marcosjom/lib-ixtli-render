//
//  ScnModelDrawCmdType.h
//  ixtli-render
//
//  Created by Marcos Ortega on 21/8/25.
//

#ifndef ScnModelDrawCmdType_h
#define ScnModelDrawCmdType_h

//ENScnModelDrawCmdType

/** @enum ENScnModelDrawCmdType
 *  @brief Model's draw commands types, by vertex or index type.
 */

typedef enum ENScnModelDrawCmdType {
    ENScnModelDrawCmdType_Undef = 0,
    //2D vertex drawing
    ENScnModelDrawCmdType_2Dv0,
    ENScnModelDrawCmdType_2Dv1,
    ENScnModelDrawCmdType_2Dv2,
    ENScnModelDrawCmdType_2Dv3,
    //2D indexed drawing
    ENScnModelDrawCmdType_2Di0,
    ENScnModelDrawCmdType_2Di1,
    ENScnModelDrawCmdType_2Di2,
    ENScnModelDrawCmdType_2Di3,
    //
    ENScnModelDrawCmdType_Count,
} ENScnModelDrawCmdType;

#endif /* ScnModelDrawCmdType_h */
