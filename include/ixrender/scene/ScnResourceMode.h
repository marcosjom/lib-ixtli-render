//
//  ScnResourceMode.h
//  nbframework
//
//  Created by Marcos Ortega on 7/8/25.
//

#ifndef ScnResourceMode_h
#define ScnResourceMode_h

#ifdef __cplusplus
extern "C" {
#endif

/** @struct ENScnResourceMode
 *  @brief This library's resources modes (buffers, textures, ...).
 */

typedef enum ENScnResourceMode {
    
    //Static resources can be modified until the
    //first time used in an ended-render-job.
    //The cpu-data is discarted after synced
    //with the gpu-data slot. This is
    //the most memory-efficient mode.
    
    ENScnResourceMode_Static = 0
    
    //DynamicSingleSlot resources only have
    //one gpu-data slot. Changes invalidated
    //in the cpu-data are synced at all
    //ended-render-job. Modifying the resource
    //while a render-pass is in progress could
    //produce artifacts. This is the second most
    //memory efficient mode but requires care of use.
   
    , ENScnResourceMode_DynamicSingleSlot
    
    //Dynamic resources can have multiple gpu-data slots.
    // Changes invalidated in the cpu-data are synced
    //to the next gpu-slot at all ended-render-job.
    //Modifying the resource while a render-pass
    //is in progress is safe since each render-pass
    //has its own gpu-data's copy. This is the safest
    //mode but consumes more memory compared to the other modes.
    
    , ENScnResourceMode_Dynamic
    
    //
    , ENScnResourceMode_Count
} ENScnResourceMode;

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnResourceMode_h */
