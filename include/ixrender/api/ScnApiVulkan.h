//
//  ScnApiVulkan.h
//  ixtli-render
//
//  Created by Marcos Ortega on 21/8/25.
//

#ifndef ScnApiVulkan_h
#define ScnApiVulkan_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/gpu/ScnGpuDevice.h"
#include "ixrender/ScnRender.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Retrieves the interface for Vulkan render API.
 * @param dst Destination for the interface.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnApiVulkan_getApiItf(STScnApiItf* dst);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiVulkan_h */
