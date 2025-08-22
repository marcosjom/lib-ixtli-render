//
//  ScnRenderJobPushMode.h
//  ixtli-render
//
//  Created by Marcos Ortega on 21/8/25.
//

#ifndef ScnRenderJobPushMode_h
#define ScnRenderJobPushMode_h

#ifdef __cplusplus
extern "C" {
#endif

/** @enum ScnRenderJobPushMode
 *  @brief Push mode for stacked objects in a commands container. Usually to detemine if the top object will result from multiplying the input or not.
 */
typedef enum ScnRenderJobPushMode {
    ScnRenderJobPushMode_Multiply = 0,  //Default, multiplies the input with the parent's state (inputs are relative properties)
    ScnRenderJobPushMode_Set,           //Ignores the parent's state and sets the input (inputs are global properties)
    //
    ScnRenderJobPushMode_Count
} ScnRenderJobPushMode;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnRenderJobPushMode_h */
