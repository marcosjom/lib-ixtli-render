//
//  ScnApiDX12.h
//  ixtli-render
//
//  Created by Marcos Ortega on 14/8/25.
//

#ifndef ScnApiDX12_h
#define ScnApiDX12_h

#include "ixrender/ixtli-defs.h"
#include "ixrender/gpu/ScnGpuDevice.h"
#include "ixrender/ScnRender.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Retrieves the interface for Microsoft's DirectX12 render API.
 * @param dst Destination for the interface.
 * @return ScnTRUE on success, ScnFALSE otherwise.
 */
ScnBOOL ScnApiDX12_getApiItf(STScnApiItf* dst);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* ScnApiDX12_h */
