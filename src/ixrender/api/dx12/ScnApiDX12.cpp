//
//  ScnApiDX12.m
//  ixtli-render
//
//  Created by Marcos Ortega on 14/8/25.
//

#include "ixrender/api/ScnApiDX12.h"
#include "ixrender/core/ScnArraySorted.h"
#include "ixrender/core/ScnMemElastic.h"
#include "ixrender/gpu/ScnGpuDevice.h"
#include "ixrender/gpu/ScnGpuBuffer.h"
#include "ixrender/gpu/ScnGpuSampler.h"
#include "ixrender/gpu/ScnGpuTexture.h"
#include "ixrender/scene/ScnRenderCmd.h"
//
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>

//using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
#include <wrl.h>

using Microsoft::WRL::ComPtr;

#ifdef SCN_ASSERTS_ACTIVATED
#include <string.h> //strncmp()
#endif

//STScnGpuDeviceApiItf

ScnGpuDeviceRef     ScnApiDX12_allocDevice(ScnContextRef ctx, const STScnGpuDeviceCfg* cfg);
void                ScnApiDX12_device_free(void* obj);
void*               ScnApiDX12_device_getApiDevice(void* obj);
STScnGpuDeviceDesc  ScnApiDX12_device_getDesc(void* obj);
ScnGpuBufferRef     ScnApiDX12_device_allocBuffer(void* obj, ScnMemElasticRef mem);
ScnGpuVertexbuffRef ScnApiDX12_device_allocVertexBuff(void* obj, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff);
ScnGpuFramebuffRef  ScnApiDX12_device_allocFramebuffFromOSView(void* obj, void* mtkView);
ScnGpuTextureRef    ScnApiDX12_device_allocTexture(void* obj, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData);
ScnGpuSamplerRef    ScnApiDX12_device_allocSampler(void* obj, const STScnGpuSamplerCfg* const cfg);
ScnGpuRenderJobRef  ScnApiDX12_device_allocRenderJob(void* obj);
//buffer
void                ScnApiDX12_buffer_free(void* data);
ScnBOOL             ScnApiDX12_buffer_sync(void* data, ScnMemElasticRef mem, const STScnGpuBufferChanges* changes);
//vertexbuff
void                ScnApiDX12_vertexbuff_free(void* data);
ScnBOOL             ScnApiDX12_vertexbuff_sync(void* data, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff);
ScnBOOL             ScnApiDX12_vertexbuff_activate(void* data);
ScnBOOL             ScnApiDX12_vertexbuff_deactivate(void* data);
//frameBuffer (view)
void                ScnApiDX12_framebuff_view_free(void* data);
STScnSize2DU        ScnApiDX12_framebuff_view_getSize(void* pObj);
ScnBOOL             ScnApiDX12_framebuff_view_syncSize(void* pObj, const STScnSize2DU size);
STScnGpuFramebuffProps ScnApiDX12_framebuff_view_getProps(void* data);
ScnBOOL             ScnApiDX12_framebuff_view_setProps(void* data, const STScnGpuFramebuffProps* const props);
//sampler
void                ScnApiDX12_sampler_free(void* pObj);
STScnGpuSamplerCfg  ScnApiDX12_sampler_getCfg(void* data);
//texture
void                ScnApiDX12_texture_free(void* pObj);
ScnBOOL             ScnApiDX12_texture_sync(void* data, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData, const STScnGpuTextureChanges* const changes);
//render job
void                ScnApiDX12_renderJob_free(void* data);
ENScnGpuRenderJobState ScnApiDX12_renderJob_getState(void* data);
ScnBOOL             ScnApiDX12_renderJob_buildBegin(void* data, ScnGpuBufferRef bPropsScns, ScnGpuBufferRef bPropsMdls);
ScnBOOL             ScnApiDX12_renderJob_buildAddCmds(void* data, const struct STScnRenderCmd* const cmds, const ScnUI32 cmdsSz);
ScnBOOL             ScnApiDX12_renderJob_buildEndAndEnqueue(void* data);

ScnBOOL ScnApiDX12_getApiItf(STScnApiItf* dst){
    if(dst == NULL) return ScnFALSE;
    //
    ScnMemory_setZeroSt(*dst);
    //gobal
    dst->allocDevice        = ScnApiDX12_allocDevice;
    //device
    dst->dev.free           = ScnApiDX12_device_free;
    dst->dev.getApiDevice   = ScnApiDX12_device_getApiDevice;
    dst->dev.getDesc        = ScnApiDX12_device_getDesc;
    dst->dev.allocBuffer    = ScnApiDX12_device_allocBuffer;
    dst->dev.allocVertexBuff = ScnApiDX12_device_allocVertexBuff;
    dst->dev.allocFramebuffFromOSView = ScnApiDX12_device_allocFramebuffFromOSView;
    dst->dev.allocTexture   = ScnApiDX12_device_allocTexture;
    dst->dev.allocSampler   = ScnApiDX12_device_allocSampler;
    dst->dev.allocRenderJob = ScnApiDX12_device_allocRenderJob;
    //buffer
    dst->buff.free          = ScnApiDX12_buffer_free;
    dst->buff.sync          = ScnApiDX12_buffer_sync;
    //vertexbuff
    dst->vertexBuff.free    = ScnApiDX12_vertexbuff_free;
    dst->vertexBuff.sync    = ScnApiDX12_vertexbuff_sync;
    dst->vertexBuff.activate = ScnApiDX12_vertexbuff_activate;
    dst->vertexBuff.deactivate = ScnApiDX12_vertexbuff_deactivate;
    //
    return ScnTRUE;
}

struct STScnApiDX12Device;

//ENScnApiDX12HeapType

typedef enum ENScnApiDX12HeapType {
    ENScnApiDX12HeapType_Buffers = 0,   //buffers
    ENScnApiDX12HeapType_Render,        //render target or depth stencil textures
    ENScnApiDX12HeapType_Textures,      //textures
    ENScnApiDX12HeapType_Samplers,      //samplers (textures)
    //
    ENScnApiDX12HeapType_Count
} ENScnApiDX12HeapType;

// STScnApiDX12HeapCfg

typedef struct STScnApiDX12HeapCfg {
    ScnMemElasticRef    mem;
    D3D12_HEAP_FLAGS    flags;
    struct STScnApiDX12Device* dev;
} STScnApiDX12HeapCfg;

void    ScnApiDX12HeapCfg_init(STScnApiDX12HeapCfg* obj);
void    ScnApiDX12HeapCfg_destroy(STScnApiDX12HeapCfg* obj);
ScnBOOL ScnApiDX12HeapCfg_prepare(STScnApiDX12HeapCfg* obj, ScnContextRef ctx, STScnMemElasticCfg* cfg, STScnMemBlockAllocItf* itf, void* itfParam);

// STScnApiDX12Heap

typedef struct STScnApiDX12Heap {
    ScnUI64         iVirtualAddr;
    ScnUI32         size;
    ID3D12Heap*     obj;
} STScnApiDX12Heap;

ScnSI32 ScnCompare_STScnApiDX12Heap(const void* data1, const void* data2, const ScnUI32 dataSz);

void* ScnApiDX12Heap_malloc(const ScnUI32 size, const char* dbgHintStr, void* itfParam);
void ScnApiDX12Heap_free(void* ptr, void* itfParam);

// STScnApiDX12Sampler

typedef struct STScnApiDX12Sampler {
    ScnContextRef           ctx;
    STScnGpuSamplerCfg      cfg;
    //id<MTLSamplerState>   smplr;
} STScnApiDX12Sampler;

//STScnApiDX12Device

typedef struct STScnApiDX12Device {
    ScnContextRef   ctx;
    STScnApiItf     itf;
    ID3D12Device*   dev;
    D3D12_FEATURE_DATA_D3D12_OPTIONS devOpts;
    ID3D12CommandQueue* cmdQueue;
    //heaps
    struct {
        ScnMutexRef mutex;
        ScnUI64     iAddrSeq;   //total ammount of bytes ever allocated, used to assign an 'unique' range of addresses to each Heap allocated
        ScnArraySortedStruct(arr, STScnApiDX12Heap);
        //byType
        STScnApiDX12HeapCfg byType[ENScnApiDX12HeapType_Count];
    } heaps;
} STScnApiDX12Device;

ScnGpuDeviceRef ScnApiDX12_allocDevice(ScnContextRef ctx, const STScnGpuDeviceCfg* cfg){
    ScnGpuDeviceRef r = ScnObjRef_Zero;
    if (ScnContext_isNull(ctx)) {
        SCN_PRINTF_ERROR("DX12, ScnApiDX12_allocDevice with null ScnContext.\n");
        return r;
    }
    UINT dxgiFactoryFlags = 0;
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
#   if defined(_DEBUG)
    {
        ComPtr<ID3D12Debug> debugController;
        if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
            SCN_PRINTF_ERROR("DX12, D3D12GetDebugInterface failed.\n");
        } else {
            debugController->EnableDebugLayer();
            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#   endif
    ScnBOOL useSoftAdapter = ScnFALSE;
    ScnBOOL requestHighPerformanceAdapter = ScnFALSE;
    ComPtr<IDXGIFactory4> factory = nullptr;
    ComPtr<ID3D12Device> dev = nullptr;
    ComPtr<ID3D12CommandQueue> cmdQueue = nullptr;
    D3D12_FEATURE_DATA_D3D12_OPTIONS devOpts;
    ScnMemory_setZeroSt(devOpts);
    //Create factory
    if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)))) {
        SCN_PRINTF_ERROR("DX12, CreateDXGIFactory2 failed.\n");
        return r;
    }
    //Open adapter and create device
    if (useSoftAdapter) {
        ComPtr<IDXGIAdapter> warpAdapter = nullptr;
        if (FAILED(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)))) {
            SCN_PRINTF_ERROR("DX12, EnumWarpAdapter failed.\n");
        } else if (S_OK != D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dev))) {
            SCN_PRINTF_ERROR("DX12, D3D12CreateDevice(warpAdapter) failed.\n");
        }
    } else {
        ComPtr<IDXGIAdapter1> hardwareAdapter = nullptr;
        ComPtr<IDXGIAdapter1> adapter = nullptr;
        ComPtr<IDXGIFactory6> factory6 = nullptr;
        //Query devices
        if (FAILED(factory->QueryInterface(IID_PPV_ARGS(&factory6)))) {
            SCN_PRINTF_ERROR("DX12, QueryInterface(IDXGIFactory6) failed.\n");
        } else {
            for (
                UINT adapterIndex = 0;
                SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                    adapterIndex,
                    requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                    IID_PPV_ARGS(&adapter)));
                ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);
                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }
                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
                    break;
                }
            }
        }
        //Query devices
        if (adapter == nullptr) {
            for (UINT adapterIndex = 0; SUCCEEDED(factory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex) {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);
                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }
                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
                    break;
                }
            }
        }
        //create device
        if (adapter == nullptr) {
            SCN_PRINTF_ERROR("DX12, no adapter found.\n");
            return r;
        } else if (FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dev)))) {
            SCN_PRINTF_ERROR("DX12, D3D12CreateDevice failed.\n");
            return r;
        }
    }
    //Query device options
    {
        if (FAILED(dev->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &devOpts, sizeof(devOpts)))) {
            SCN_PRINTF_ERROR("DX12, CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS) failed.\n");
                return r;
        }
    }
    //Create command queue
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        //
        if (FAILED(dev->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cmdQueue)))) {
            SCN_PRINTF_ERROR("DX12, CreateCommandQueue failed.\n");
            return r;
        }
    }
    //Ceate
    SCN_ASSERT(dev.Get() != nullptr);
    SCN_ASSERT(cmdQueue.Get() != nullptr);
    STScnApiDX12Device* obj = (STScnApiDX12Device*)ScnContext_malloc(ctx, sizeof(STScnApiDX12Device), SCN_DBG_STR("STScnApiDX12Device"));
    if (obj == NULL) {
        SCN_PRINTF_ERROR("DX12, ScnContext_malloc(STScnApiDX12Device) failed.\n");
    } else {
        //init STScnApiDX12Device
        ScnMemory_setZeroSt(*obj);
        ScnContext_set(&obj->ctx, ctx);
        obj->dev = dev.Detach();
        obj->devOpts = devOpts;
        obj->cmdQueue = cmdQueue.Detach();
        //heaps
        {
            //init (heaps)
            obj->heaps.mutex    = ScnContext_allocMutex(ctx);
            obj->heaps.iAddrSeq = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
            ScnArraySorted_init(ctx, &obj->heaps.arr, 0, 16, STScnApiDX12Heap, ScnCompare_STScnApiDX12Heap);
            //by type
            {
                ScnSI32 i; for (i = 0; i < sizeof(obj->heaps.byType) / sizeof(obj->heaps.byType[0]); ++i) {
                    STScnApiDX12HeapCfg* cfg = &obj->heaps.byType[i];
                    ScnApiDX12HeapCfg_init(cfg);
                    cfg->dev = obj;
                }
            }
            //configure (heaps)
            STScnMemBlockAllocItf heapItf;
            heapItf.malloc  = ScnApiDX12Heap_malloc;
            heapItf.free    = ScnApiDX12Heap_free;
            //
            switch (devOpts.ResourceHeapTier) {
                case D3D12_RESOURCE_HEAP_TIER_1:
                    //Three different heaps: (1)buffers, (2)render target or depth stencil textures, (3) other textures
                    //D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS
                    SCN_PRINTF_WARNING("DX12,Device is D3D12_RESOURCE_HEAP_TIER_1; heaps must be divided by types.\n");
                    //configure buffers-heap
                    {
                        ENScnApiDX12HeapType type = ENScnApiDX12HeapType_Buffers;
                        STScnMemElasticCfg mCfg = STScnMemElasticCfg_Zero;
                        mCfg.idxsAlign = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT; //32
                        mCfg.sizeAlign = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
                        mCfg.sizePerBlock = (1024 * 1024 * 4) / mCfg.sizeAlign * mCfg.sizeAlign;
                        mCfg.sizeInitial = 0; //do not allocate memory yet
                        mCfg.isIdxZeroValid = ScnTRUE;
                        STScnApiDX12HeapCfg* cfg = &obj->heaps.byType[type];
                        cfg->flags = D3D12_HEAP_FLAG_NONE;
                        cfg->dev = obj;
                        if (!ScnApiDX12HeapCfg_prepare(cfg, ctx, &mCfg, &heapItf, obj)) {
                            SCN_PRINTF_ERROR("DX12, ScnApiDX12HeapCfg_prepare failed.\n");
                        }
                    }
                    {
                        ENScnApiDX12HeapType type = ENScnApiDX12HeapType_Render;
                        STScnMemElasticCfg mCfg = STScnMemElasticCfg_Zero;
                        mCfg.idxsAlign = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT; //32
                        mCfg.sizeAlign = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
                        mCfg.sizePerBlock = (1024 * 1024 * 4) / mCfg.sizeAlign * mCfg.sizeAlign;
                        mCfg.sizeInitial = 0; //do not allocate memory yet
                        mCfg.isIdxZeroValid = ScnTRUE;
                        STScnApiDX12HeapCfg* cfg = &obj->heaps.byType[type];
                        cfg->flags = D3D12_HEAP_FLAG_NONE;
                        cfg->dev = obj;
                        if (!ScnApiDX12HeapCfg_prepare(cfg, ctx, &mCfg, &heapItf, obj)) {
                            SCN_PRINTF_ERROR("DX12, ScnApiDX12HeapCfg_prepare failed.\n");
                        }
                    }
                    {
                        ENScnApiDX12HeapType type = ENScnApiDX12HeapType_Textures;
                        STScnMemElasticCfg mCfg = STScnMemElasticCfg_Zero;
                        mCfg.idxsAlign = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT; //32
                        mCfg.sizeAlign = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
                        mCfg.sizePerBlock = (1024 * 1024 * 4) / mCfg.sizeAlign * mCfg.sizeAlign;
                        mCfg.sizeInitial = 0; //do not allocate memory yet
                        mCfg.isIdxZeroValid = ScnTRUE;
                        STScnApiDX12HeapCfg* cfg = &obj->heaps.byType[type];
                        cfg->flags = D3D12_HEAP_FLAG_NONE;
                        cfg->dev = obj;
                        if (!ScnApiDX12HeapCfg_prepare(cfg, ctx, &mCfg, &heapItf, obj)) {
                            SCN_PRINTF_ERROR("DX12, ScnApiDX12HeapCfg_prepare failed.\n");
                        }
                    }
                    //configure other heaps to use textures-heap
                    {
                        ENScnApiDX12HeapType type = ENScnApiDX12HeapType_Textures;
                        STScnApiDX12HeapCfg* cfg2 = &obj->heaps.byType[type];
                        ScnSI32 i; for (i = 0; i < sizeof(obj->heaps.byType) / sizeof(obj->heaps.byType[0]); ++i) {
                            STScnApiDX12HeapCfg* cfg = &obj->heaps.byType[i];
                            if (!ScnMemElastic_isNull(cfg->mem)) {
                                continue;
                            }
                            cfg->flags = cfg2->flags;
                            cfg->dev = obj;
                            ScnMemElastic_set(&cfg->mem, cfg2->mem);
                        }
                    }
                    break;
                default: //D3D12_RESOURCE_HEAP_TIER_2
                    //Heaps can host any type of objects
                    SCN_PRINTF_INFO("DX12,Device is >= D3D12_RESOURCE_HEAP_TIER_2; heaps are universal.\n");
                    {
                        //configure buffers-heap
                        {
                            ENScnApiDX12HeapType type = ENScnApiDX12HeapType_Buffers;
                            STScnMemElasticCfg mCfg = STScnMemElasticCfg_Zero;
                            mCfg.idxsAlign = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT; //32
                            mCfg.sizeAlign = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
                            mCfg.sizePerBlock = (1024 * 1024 * 4) / mCfg.sizeAlign * mCfg.sizeAlign;
                            mCfg.sizeInitial = 0; //do not allocate memory yet
                            mCfg.isIdxZeroValid = ScnTRUE;
                            //
                            STScnApiDX12HeapCfg* cfg = &obj->heaps.byType[type];
                            cfg->flags = D3D12_HEAP_FLAG_NONE;
                            cfg->dev = obj;
                            if(!ScnApiDX12HeapCfg_prepare(cfg, ctx, &mCfg, &heapItf, obj)){
                                SCN_PRINTF_ERROR("DX12, ScnApiDX12HeapCfg_prepare failed.\n");
                            }
                        }
                        //configure other heaps to use buffers-heap
                        {
                            STScnApiDX12HeapCfg* cfg2 = &obj->heaps.byType[ENScnApiDX12HeapType_Buffers];
                            ScnSI32 i; for (i = 0; i < sizeof(obj->heaps.byType) / sizeof(obj->heaps.byType[0]); ++i) {
                                STScnApiDX12HeapCfg* cfg = &obj->heaps.byType[i];
                                if (!ScnMemElastic_isNull(cfg->mem)) {
                                    continue;
                                }
                                cfg->flags = cfg2->flags;
                                cfg->dev = obj;
                                ScnMemElastic_set(&cfg->mem, cfg2->mem);
                            }
                        }
                    }
                    break;
            }
        }
        //
        if (!ScnApiDX12_getApiItf(&obj->itf)) {
            SCN_PRINTF_ERROR("ScnApiDX12_allocDevice::ScnApiDX12_getApiItf failed.\n");
        } else {
            ScnGpuDeviceRef d = ScnGpuDevice_alloc(ctx);
            if (!ScnGpuDevice_isNull(d)) {
                if (!ScnGpuDevice_prepare(d, &obj->itf.dev, obj)) {
                    SCN_PRINTF_ERROR("ScnApiDX12_allocDevice::ScnGpuDevice_prepare failed.\n");
                } else {
                    ScnGpuDevice_set(&r, d);
                    obj = NULL; //consume
                }
                ScnGpuDevice_releaseAndNull(&d);
            }
        }
    }
    //release (if not consumed)
    if (obj != NULL) {
        ScnApiDX12_device_free(obj);
        obj = NULL;
    }
    //
    return r;
}

void ScnApiDX12_device_free(void* pObj){
    STScnApiDX12Device* obj = (STScnApiDX12Device*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        //heaps
        {
            //by type
            {
                ScnSI32 i; for (i = 0; i < sizeof(obj->heaps.byType) / sizeof(obj->heaps.byType[0]); ++i) {
                    STScnApiDX12HeapCfg* cfg = &obj->heaps.byType[i];
                    ScnApiDX12HeapCfg_destroy(cfg);
                }
            }
            SCN_ASSERT(obj->heaps.arr.use == 0) //all heaps should be already destroyed
            ScnArraySorted_empty(&obj->heaps.arr);
            ScnArraySorted_destroy(ctx, &obj->heaps.arr);
            ScnMutex_free(&obj->heaps.mutex);
        }
        if(obj->cmdQueue != nullptr){
            obj->cmdQueue->Release();
            obj->cmdQueue = nullptr;
        }
        if(obj->dev != nullptr){
            obj->dev->Release();
            obj->dev = nullptr;
        }
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

// STScnApiDX12HeapCfg

void ScnApiDX12HeapCfg_init(STScnApiDX12HeapCfg* obj) {
    ScnMemory_setZeroSt(*obj);
}

void ScnApiDX12HeapCfg_destroy(STScnApiDX12HeapCfg* obj) {
    ScnMemElastic_releaseAndNull(&obj->mem);
    obj->flags = D3D12_HEAP_FLAG_NONE;
    obj->dev = NULL;
}

ScnBOOL ScnApiDX12HeapCfg_prepare(STScnApiDX12HeapCfg* obj, ScnContextRef ctx, STScnMemElasticCfg* cfg, STScnMemBlockAllocItf* itf, void* itfParam) {
    ScnBOOL r = ScnFALSE;
    ScnMemElasticRef mem = ScnMemElastic_alloc(ctx);
    if (!ScnMemElastic_isNull(mem)) {
        SCN_PRINTF_ERROR("DX12,ScnMemElastic_alloc failed.\n");
    } else if (!ScnMemElastic_prepare(mem, cfg, itf, itfParam, NULL)) {
        SCN_PRINTF_ERROR("DX12,ScnMemElastic_prepare failed.\n");
    } else {
        ScnMemElastic_set(&obj->mem, mem);
        r = ScnTRUE;
    }
    ScnMemElastic_releaseAndNull(&mem);
    return r;
}

// STScnApiDX12Heap

ScnSI32 ScnCompare_STScnApiDX12Heap(const void* data1, const void* data2, const ScnUI32 dataSz) {
    SCN_ASSERT(dataSz == sizeof(STScnApiDX12Heap))
    if (dataSz == sizeof(STScnApiDX12Heap)) {
        const STScnApiDX12Heap* d1 = (STScnApiDX12Heap*)data1;
        const STScnApiDX12Heap* d2 = (STScnApiDX12Heap*)data2;
        return (d1->iVirtualAddr < d2->iVirtualAddr ? -1 : d1->iVirtualAddr > d2->iVirtualAddr ? 1 : 0);
    }
    return -2;
}

void* ScnApiDX12Heap_malloc(const ScnUI32 size, const char* dbgHintStr, void* itfParam) {
    void* r = NULL;
    STScnApiDX12HeapCfg* cfg = (STScnApiDX12HeapCfg*)itfParam;
    if (cfg == NULL || cfg->dev == nullptr || size <= 0) { //zero-sz heaps are not allowed
        return r;
    }
    SCN_ASSERT((size % D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) == 0) //user missconfiguration
    if ((size % D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) != 0) { //heap must be aligned
        return r;
    }
    STScnApiDX12Device* obj = cfg->dev;
    ScnMutex_lock(obj->heaps.mutex);
    {
        //allocate new heap
        D3D12_HEAP_DESC heapDesc = {};
        heapDesc.SizeInBytes = size;
        heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        heapDesc.Flags = D3D12_HEAP_FLAG_CREATE_NOT_ZEROED | cfg->flags;
        /*
            D3D12_HEAP_FLAG_NONE = 0,
            D3D12_HEAP_FLAG_SHARED = 0x1,
            D3D12_HEAP_FLAG_DENY_BUFFERS = 0x4,
            D3D12_HEAP_FLAG_ALLOW_DISPLAY = 0x8,
            D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER = 0x20,
            D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES = 0x40,
            D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES = 0x80,
            D3D12_HEAP_FLAG_HARDWARE_PROTECTED = 0x100,
            D3D12_HEAP_FLAG_ALLOW_WRITE_WATCH = 0x200,
            D3D12_HEAP_FLAG_ALLOW_SHADER_ATOMICS = 0x400,
            D3D12_HEAP_FLAG_CREATE_NOT_RESIDENT = 0x800,
            D3D12_HEAP_FLAG_CREATE_NOT_ZEROED = 0x1000,
            D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES = 0,
            D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS = 0xc0,
            D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES = 0x44,
            D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES = 0x84
            */
        //add record
        ID3D12Heap* heap = nullptr;
        if(FAILED(obj->dev->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap)))) {
            SCN_PRINTF_ERROR("DX12, ScnApiDX12Heap_malloc::CreateHeap(%u bytes) failed.\n", size);
        } else {
            STScnApiDX12Heap h;
            ScnMemory_setZeroSt(h);
            h.iVirtualAddr  = obj->heaps.iAddrSeq;
            h.obj           = heap;
            h.size          = size;
            if(NULL == ScnArraySorted_addPtr(obj->ctx, &obj->heaps.arr, &h, STScnApiDX12Heap)) {
                SCN_PRINTF_ERROR("DX12, ScnApiDX12Heap_malloc::ScnArraySorted_addPtr(%u bytes) failed.\n", size);
            } else {
                SCN_PRINTF_INFO("DX12, ScnApiDX12Heap_malloc::(%u bytes) success at v-address: %llu.\n", size, h.iVirtualAddr);
                heap = nullptr; //consume
                r = (void*)h.iVirtualAddr;
                obj->heaps.iAddrSeq += size;
            }
        }
        //release (if not consumed)
        if (heap != nullptr) {
            heap->Release();
            heap = nullptr;
        }
    }
    ScnMutex_unlock(obj->heaps.mutex);
    return r;
}

void ScnApiDX12Heap_free(void* ptr, void* itfParam) {
    STScnApiDX12Device* dev = (STScnApiDX12Device*)itfParam;
    if (dev == NULL || dev->dev == nullptr || ptr == NULL) {
        return;
    }
    ScnMutex_lock(dev->heaps.mutex);
    {
        STScnApiDX12Heap srch;
        srch.iVirtualAddr = (ScnUI64)ptr;
        const ScnSI32 iFnd = ScnArraySorted_indexOf(&dev->heaps.arr, &srch);
        SCN_ASSERT(iFnd >= 0) //should be found
        if (iFnd >= 0) {
            //destroy heap
            STScnApiDX12Heap* h = &dev->heaps.arr.arr[iFnd];
            SCN_PRINTF_INFO("DX12, ScnApiDX12Heap_malloc::(%u bytes) success.\n", h->size);
            if (h->obj != nullptr) {
                h->obj->Release();
                h->obj = nullptr;
            }
            //remove record
            ScnArraySorted_removeItemAtIndex(&dev->heaps.arr, iFnd);
        }
    }
    ScnMutex_unlock(dev->heaps.mutex);
}

// STScnApiDX12Sampler

ScnGpuSamplerRef ScnApiDX12_device_allocSampler(void* pObj, const STScnGpuSamplerCfg* const cfg){
    ScnGpuSamplerRef r = ScnGpuSamplerRef_Zero;
    STScnApiDX12Device* dev = (STScnApiDX12Device*)pObj;
    if (dev == NULL || dev->dev == nullptr || cfg == NULL) {
        SCN_PRINTF_ERROR("ScnApiDX12_device_allocSampler missing params.\n");
        return r;
    }
    //sampler description
    D3D12_SAMPLER_DESC sDesc = {};
    sDesc.Filter = (
        cfg->minFilter == ENScnGpuSamplerFilter_Linear && cfg->magFilter == ENScnGpuSamplerFilter_Linear ? D3D12_FILTER_MIN_MAG_MIP_LINEAR :
        cfg->minFilter == ENScnGpuSamplerFilter_Linear && cfg->magFilter == ENScnGpuSamplerFilter_Nearest ? D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT :
        cfg->minFilter == ENScnGpuSamplerFilter_Nearest && cfg->magFilter == ENScnGpuSamplerFilter_Linear ? D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR :
        D3D12_FILTER_MIN_MAG_MIP_POINT
    );
    sDesc.AddressU = sDesc.AddressV = sDesc.AddressW = (
        cfg->address == ENScnGpusamplerAddress_Clamp ? D3D12_TEXTURE_ADDRESS_MODE_BORDER : D3D12_TEXTURE_ADDRESS_MODE_WRAP
        );
    sDesc.MipLODBias = 0;
    sDesc.MaxAnisotropy = 0;
    sDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    //sDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK; //static_sampler_only
    sDesc.MinLOD = 0.0f;
    sDesc.MaxLOD = D3D12_FLOAT32_MAX;
    //sDesc.ShaderRegister = 0; //static_sampler_only
    //sDesc.RegisterSpace = 0; //static_sampler_only
    //sDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //static_sampler_only
    //create sampler
    const ScnUI32 sz = dev->dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    STScnApiDX12HeapCfg* heap = &dev->heaps.byType[ENScnApiDX12HeapType_Samplers];
    STScnAbsPtr ptr = ScnMemElastic_malloc(heap->mem, sz, NULL);
    if (ptr.ptr == NULL) {
        SCN_PRINTF_ERROR("DX12, ScnApiDX12_device_allocSampler, ScnMemElastic_malloc(%u) failed.\n", sz);
        return r;
    }
    SCN_ASSERT(ptr.itfParam != NULL);
    STScnApiDX12Heap* heap = (STScnApiDX12Heap*)ptr.itfParam;
    if (dev->dev->CreateSampler()) {


        ScnMutexRef mutex;
        ScnUI64     iAddrSeq;   //total ammount of bytes ever allocated, used to assign an 'unique' range of addresses to each Heap allocated
        ScnArraySortedStruct(arr, STScnApiDX12Heap);
        //byType
        STScnApiDX12HeapCfg byType[ENScnApiDX12HeapType_Count];
    }
    //ID3D12SamplerState* sampler = nullptr;
    

    /*
    {
        MTLSamplerDescriptor* desc = [MTLSamplerDescriptor new];
        if(desc == nullptr){
            SCN_PRINTF_ERROR("[MTLSamplerDescriptor new] failed.\n");
        } else {
            const MTLSamplerAddressMode addressMode = (cfg->address == ENScnGpusamplerAddress_Clamp ? MTLSamplerAddressModeClampToEdge : MTLSamplerAddressModeRepeat);
            desc.minFilter = (cfg->magFilter == ENScnGpuSamplerFilter_Linear ? MTLSamplerMinMagFilterLinear : MTLSamplerMinMagFilterNearest);
            desc.magFilter = (cfg->minFilter == ENScnGpuSamplerFilter_Linear ? MTLSamplerMinMagFilterLinear : MTLSamplerMinMagFilterNearest);
            desc.sAddressMode = desc.tAddressMode = desc.rAddressMode = addressMode;
            id<MTLSamplerState> sampler = [dev->dev newSamplerStateWithDescriptor:desc];
            if(sampler == nullptr){
                SCN_PRINTF_ERROR("[dev newSamplerStateWithDescriptor] failed.\n");
            } else {
                STScnApiDX12Sampler* obj = (STScnApiDX12Sampler*)ScnContext_malloc(dev->ctx, sizeof(STScnApiDX12Sampler), SCN_DBG_STR("STScnApiDX12Sampler"));
                if(obj == NULL){
                    SCN_PRINTF_ERROR("ScnContext_malloc(STScnApiDX12Sampler) failed.\n");
                } else {
                    ScnMemory_setZeroSt(*obj);
                    ScnContext_set(&obj->ctx, dev->ctx);
                    obj->cfg    = *cfg;
                    obj->smplr  = sampler; [sampler retain];
                    //
                    ScnGpuSamplerRef s = ScnGpuSampler_alloc(dev->ctx);
                    if(!ScnGpuSampler_isNull(s)){
                        STScnGpuSamplerApiItf itf;
                        ScnMemory_setZeroSt(itf);
                        itf.free    = ScnApiDX12_sampler_free;
                        itf.getCfg  = ScnApiDX12_sampler_getCfg;
                        if(!ScnGpuSampler_prepare(s, &itf, obj)){
                            SCN_PRINTF_ERROR("ScnApiDX12_device_allocSampler::ScnGpuSampler_prepare failed.\n");
                        } else {
                            ScnGpuSampler_set(&r, s);
                            obj = NULL; //consume
                        }
                        ScnGpuSampler_releaseAndNull(&s);
                    }
                }
                //release (if not consumed)
                if(obj != NULL){
                    ScnApiDX12_sampler_free(obj);
                    obj = NULL;
                }
                //
                [sampler release];
                sampler = nullptr;
            }
            //
            [desc release];
            desc = nullptr;
        }
    }*/
    return r;
}

void ScnApiDX12_sampler_free(void* pObj){
    STScnApiDX12Sampler* obj = (STScnApiDX12Sampler*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        /*if (obj->smplr != nullptr) {
            [obj->smplr release];
            obj->smplr = nullptr;
        }*/
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

STScnGpuSamplerCfg ScnApiDX12_sampler_getCfg(void* pObj){
    STScnGpuSamplerCfg r = STScnGpuSamplerCfg_Zero;
    STScnApiDX12Sampler* obj = (STScnApiDX12Sampler*)pObj;
    if(obj != NULL ){
        r = obj->cfg;
    }
    return r;
}

//STScnApiDX12Buffer

typedef struct STScnApiDX12Buffer {
    ScnContextRef   ctx;
    STScnApiItf     itf;
    id<MTLDevice>   dev;
    id<MTLBuffer>   buff;
} STScnApiDX12Buffer;

void* ScnApiDX12_device_getApiDevice(void* pObj){
    STScnApiDX12Device* dev = (STScnApiDX12Device*)pObj;
    return (dev != NULL ? dev->dev : NULL);
}

//#define	D3D12_CS_4_X_RAW_UAV_BYTE_ALIGNMENT	( 256 )
//#define	D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT	( 16 )
//#define	D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT	( 65536 )

STScnGpuDeviceDesc ScnApiDX12_device_getDesc(void* obj){
    STScnGpuDeviceDesc r = STScnGpuDeviceDesc_Zero;
    {
        r.isTexFmtInfoRequired  = ScnTRUE;  //fragment shader requires the textures format info to produce correct color output
        r.offsetsAlign          = 32;       //buffers offsets aligment
        r.memBlockAlign         = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;      //buffer memory copy alignment
    }
    return r;
}

ScnBOOL ScnApiDX12_buffer_syncRanges_(id<MTLBuffer> buff, ScnMemElasticRef mem, const STScnRangeU* const rngs, const ScnUI32 rngsUse);

ScnGpuBufferRef ScnApiDX12_device_allocBuffer(void* pObj, ScnMemElasticRef mem){
    ScnGpuBufferRef r = ScnObjRef_Zero;
    STScnApiDX12Device* dev = (STScnApiDX12Device*)pObj;
    if(dev != NULL && dev->dev != NULL && !ScnMemElastic_isNull(mem)){
        const ScnUI32 cpuBuffSz = ScnMemElastic_getAddressableSize(mem);
        if(cpuBuffSz <= 0){
            SCN_PRINTF_ERROR("allocating zero-sz gpu buffer is not allowed.\n");
        } else {
            id<MTLBuffer> buff = [dev->dev newBufferWithLength:cpuBuffSz options:MTLResourceStorageModeShared | MTLResourceCPUCacheModeWriteCombined];
            if(buff != nullptr){
                const STScnRangeU rngAll = ScnMemElastic_getUsedAddressesRngAligned(mem);
                if(!ScnApiDX12_buffer_syncRanges_(buff, mem, &rngAll, 1)){
                    SCN_PRINTF_ERROR("ScnApiDX12_buffer_syncRanges_ failed.\n");
                } else {
                    //synced
                    STScnApiDX12Buffer* obj = (STScnApiDX12Buffer*)ScnContext_malloc(dev->ctx, sizeof(STScnApiDX12Buffer), SCN_DBG_STR("STScnApiDX12Buffer"));
                    if(obj == NULL){
                        SCN_PRINTF_ERROR("ScnContext_malloc(STScnApiDX12Buffer) failed.\n");
                    } else {
                        ScnMemory_setZeroSt(*obj);
                        ScnContext_set(&obj->ctx, dev->ctx);
                        obj->itf        = dev->itf;
                        obj->dev        = dev->dev;  //retain?
                        obj->buff       = buff; [buff retain];
                        //
                        ScnGpuBufferRef d = ScnGpuBuffer_alloc(dev->ctx);
                        if(!ScnGpuBuffer_isNull(d)){
                            if(!ScnGpuBuffer_prepare(d, &obj->itf.buff, obj)){
                                SCN_PRINTF_ERROR("ScnApiDX12_allocDevice::ScnGpuBuffer_prepare failed.\n");
                            } else {
                                ScnGpuBuffer_set(&r, d);
                                obj = NULL; //consume
                            }
                            ScnGpuBuffer_releaseAndNull(&d);
                        }
                    }
                    //release (if not consumed)
                    if(obj != NULL){
                        ScnApiDX12_buffer_free(obj);
                        obj = NULL;
                    }
                }
                [buff release];
                buff = nullptr;
            }
        }
    }
    return r;
}

void ScnApiDX12_buffer_free(void* pObj){
    STScnApiDX12Buffer* obj = (STScnApiDX12Buffer*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        if(obj->buff != nullptr){
            [obj->buff release];
            obj->buff = nullptr;
        }
        if(obj->dev != nullptr){
            //release?
            obj->dev = nullptr;
        }
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

#ifdef SCN_ASSERTS_ACTIVATED
ScnBOOL ScnApiDX12_buffer_syncValidate_(STScnApiDX12Buffer* obj, ScnMemElasticRef mem);
#endif

ScnBOOL ScnApiDX12_buffer_sync(void* pObj, ScnMemElasticRef mem, const STScnGpuBufferChanges* changes){
    ScnBOOL r = ScnFALSE;
    STScnApiDX12Buffer* obj = (STScnApiDX12Buffer*)pObj;
    if(obj->buff == nullptr || ScnMemElastic_isNull(mem)){
        //missing params
        return ScnFALSE;
    }
    //sync
    {
        ScnBOOL buffIsNew = ScnFALSE;
        ScnUI32 buffLen = (ScnUI32)obj->buff.length;
        const ScnUI32 cpuBuffSz = ScnMemElastic_getAddressableSize(mem);
        //resize (if necesary)
        if(cpuBuffSz != buffLen){
            //recreate buffer
            id<MTLBuffer> buff = [obj->dev newBufferWithLength:cpuBuffSz options:MTLResourceStorageModeShared | MTLResourceCPUCacheModeWriteCombined];
            if(buff != nullptr){
                SCN_PRINTF_VERB("ScnApiDX12_buffer_sync::gpu-buff resized from %u to %u bytes.\n", buffLen, cpuBuffSz);
                buffLen = cpuBuffSz;
                [obj->buff release];
                obj->buff = buff;
                buffIsNew = ScnTRUE;
            }
        }
        //sync
        if(buffLen < cpuBuffSz){
            SCN_PRINTF_ERROR("ScnApiDX12_buffer_sync::gpuBuff is smaller than cpu-buff.\n");
        } else if(buffIsNew || changes->all){
            //sync all
            const STScnRangeU rngAll = ScnMemElastic_getUsedAddressesRngAligned(mem);
            if(!ScnApiDX12_buffer_syncRanges_(obj->buff, mem, &rngAll, 1)){
                SCN_PRINTF_ERROR("ScnApiDX12_buffer_sync::ScnApiDX12_buffer_syncRanges_ failed.\n");
            } else {
                r = ScnTRUE;
                //validate
                //SCN_ASSERT(ScnApiDX12_buffer_syncValidate_(obj, mem))
            }
        } else {
            //sync ranges only
            if(!ScnApiDX12_buffer_syncRanges_(obj->buff, mem, changes->rngs, changes->rngsUse)){
                SCN_PRINTF_ERROR("ScnApiDX12_buffer_sync::ScnApiDX12_buffer_syncRanges_ failed.\n");
            } else {
                r = ScnTRUE;
                //validate
                //SCN_ASSERT(ScnApiDX12_buffer_syncValidate_(obj, mem))
            }
        }
    }
    return r;
}

#ifdef SCN_ASSERTS_ACTIVATED
typedef struct ScnApiDX12_buffer_syncValidate_st {
    STScnApiDX12Buffer*    obj;
    STScnRangeU             usedAddressesRng;
    ScnUI32                 leftLimitFnd;
    ScnUI32                 rghtLimitFnd;
} ScnApiDX12_buffer_syncValidate_st;
#endif

#ifdef SCN_ASSERTS_ACTIVATED
ScnBOOL ScnApiDX12_buffer_syncValidate_pushBlockPtrs_(void* data, const ScnUI32 rootIndex, const void* rootAddress, const STScnMemBlockPtr* const ptrs, const ScnUI32 ptrsSz){
    ScnBOOL r = ScnFALSE;
    ScnApiDX12_buffer_syncValidate_st* st = (ScnApiDX12_buffer_syncValidate_st*)data;
    STScnApiDX12Buffer* obj = st->obj;
    const STScnRangeU usedAddressesRng = st->usedAddressesRng;
    if(obj->buff != nullptr){
        const ScnUI32 buffLen       = (ScnUI32)obj->buff.length;
        ScnBYTE* buffPtr            = (ScnBYTE*)obj->buff.contents;
        const STScnMemBlockPtr* ptr = ptrs;
        const STScnMemBlockPtr* ptrAfterEnd = ptr + ptrsSz;
        const ScnUI32 usedAddressesRngAfterEnd = usedAddressesRng.start + usedAddressesRng.size;
        r = ScnTRUE;
        while(ptr < ptrAfterEnd){
            const ScnUI32 ptrIdx = rootIndex + (ScnUI32)((const ScnBYTE*)ptr->ptr - (const ScnBYTE*)rootAddress);
            const ScnUI32 ptrIdxAfterEnd = ptrIdx + ptr->sz;
            if(ptrIdx < usedAddressesRng.start || ptrIdxAfterEnd > usedAddressesRngAfterEnd){
                SCN_ASSERT(ScnFALSE) //pointer out of ScnMemElastic's declared acitve range (program logic error)
                r = ScnFALSE;
                break;
            } else if(ptrIdxAfterEnd > buffLen){
                SCN_ASSERT(ScnFALSE) //pointer out of buffer's range
                r = ScnFALSE;
                break;
            } else if(ptr->sz > 0 && 0 != strncmp((const char*)&buffPtr[ptrIdx], ptr->ptr, ptr->sz)){
                SCN_ASSERT(ScnFALSE) //data missmatch
                r = ScnFALSE;
                break;
            }
            if(ptrIdx <= st->leftLimitFnd){
                SCN_ASSERT(ptrIdx < st->leftLimitFnd) //ptrs should not repeat themself
                st->leftLimitFnd = ptrIdx;
            }
            if(ptrIdxAfterEnd >= st->rghtLimitFnd){
                SCN_ASSERT(ptrIdxAfterEnd > st->rghtLimitFnd) //ptrs should not repeat themself
                st->rghtLimitFnd = ptrIdxAfterEnd;
            }
            //next
            ++ptr;
        }
    }
    return r;
}
#endif

#ifdef SCN_ASSERTS_ACTIVATED
ScnBOOL ScnApiDX12_buffer_syncValidate_(STScnApiDX12Buffer* obj, ScnMemElasticRef mem){
    ScnBOOL r = ScnTRUE;
    ScnApiDX12_buffer_syncValidate_st st;
    ScnMemory_setZeroSt(st);
    st.obj              = obj;
    st.usedAddressesRng = ScnMemElastic_getUsedAddressesRng(mem);
    st.leftLimitFnd     = 0xFFFFFFFFu;
    st.rghtLimitFnd     = 0;
    {
        r = ScnMemElastic_pushPtrs(mem, ScnApiDX12_buffer_syncValidate_pushBlockPtrs_, &st);
    }
    SCN_ASSERT(st.leftLimitFnd == st.usedAddressesRng.start && st.rghtLimitFnd == (st.usedAddressesRng.start + st.usedAddressesRng.size)) //program logic error, the used-address-rng notified by the elastic-memory was miscalculated
    return r;
}
#endif

ScnBOOL ScnApiDX12_buffer_syncRanges_(id<MTLBuffer> buff, ScnMemElasticRef mem, const STScnRangeU* const rngs, const ScnUI32 rngsUse){
    ScnBOOL r = ScnTRUE;
    if(rngsUse <= 0){
        //nothing to sync
        return r;
    }
    //
    const ScnUI32 buffLen = (ScnUI32)buff.length;
    ScnBYTE* buffPtr = (ScnBYTE*)buff.contents;
    STScnAbsPtr ptr = STScnAbsPtr_Zero;
    ScnUI32 continuousSz = 0, copySz = 0, bytesCopied = 0;
    //
#   ifdef SCN_ASSERTS_ACTIVATED
    ScnUI32 prevRngAfterEnd = 0;
#   endif
    const STScnRangeU* rng = rngs;
    const STScnRangeU* rngAfterEnd = rngs + rngsUse;
    STScnRangeU curRng;
    while(rng < rngAfterEnd && r){
        SCN_ASSERT(prevRngAfterEnd <= rng->start + rng->size) //rngs should be ordered and non-overlapping
        //copy range data
        curRng = *rng;
        while(curRng.size > 0){
            ptr = ScnMemElastic_getNextContinuousAddress(mem, curRng.start, &continuousSz);
            if(ptr.ptr == NULL){
                break;
            }
            SCN_ASSERT(ptr.idx == curRng.start);
            SCN_ASSERT((curRng.start + continuousSz) <= buffLen);
            if((curRng.start + continuousSz) > buffLen){
                SCN_PRINTF_ERROR("gpu-buffer is smaller than cpu-buffer.\n");
                r = ScnFALSE;
                break;
            }
            //copy
            copySz = (curRng.size < continuousSz ? curRng.size : continuousSz);
            ScnMemcpy(&buffPtr[curRng.start], ptr.ptr, copySz);
            bytesCopied += copySz;
            //
            SCN_ASSERT(curRng.size >= copySz)
            curRng.start += copySz;
            curRng.size -= copySz;
        }
#       ifdef SCN_ASSERTS_ACTIVATED
        prevRngAfterEnd = rng->start + rng->size;
#       endif
        //next
        ++rng;
    }
    SCN_PRINTF_VERB("%2.f%% %u of %u bytes synced at buffer.\n", (float)bytesCopied * 100.f / (float)buffLen, bytesCopied, buffLen);
    return r;
}

//STScnApiDX12VertexBuffer

typedef struct STScnApiDX12VertexBuff {
    ScnContextRef           ctx;
    STScnGpuVertexbuffCfg   cfg;
    STScnApiItf             itf;
    ScnGpuBufferRef         vBuff;
    ScnGpuBufferRef         idxBuff;
} STScnApiDX12VertexBuff;

ScnGpuVertexbuffRef ScnApiDX12_device_allocVertexBuff(void* pObj, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff){
    ScnGpuVertexbuffRef r = ScnObjRef_Zero;
    STScnApiDX12Device* dev = (STScnApiDX12Device*)pObj;
    if(dev != NULL && dev->dev != NULL && cfg != NULL && !ScnGpuBuffer_isNull(vBuff)){ //idxBuff is optional
        //synced
        STScnApiDX12VertexBuff* obj = (STScnApiDX12VertexBuff*)ScnContext_malloc(dev->ctx, sizeof(STScnApiDX12VertexBuff), SCN_DBG_STR("STScnApiDX12VertexBuff"));
        if(obj == NULL){
            SCN_PRINTF_ERROR("ScnContext_malloc(STScnApiDX12VertexBuff) failed.\n");
        } else {
            ScnMemory_setZeroSt(*obj);
            ScnContext_set(&obj->ctx, dev->ctx);
            obj->itf        = dev->itf;
            obj->cfg        = *cfg;
            ScnGpuBuffer_set(&obj->vBuff, vBuff);
            ScnGpuBuffer_set(&obj->idxBuff, idxBuff);
            //
            ScnGpuVertexbuffRef d = ScnGpuVertexbuff_alloc(dev->ctx);
            if(!ScnGpuVertexbuff_isNull(d)){
                if(!ScnGpuVertexbuff_prepare(d, &obj->itf.vertexBuff, obj)){
                    SCN_PRINTF_ERROR("ScnApiDX12_device_allocVertexBuff::ScnGpuVertexbuff_prepare failed.\n");
                } else {
                    ScnGpuVertexbuff_set(&r, d);
                    obj = NULL; //consume
                }
                ScnGpuVertexbuff_releaseAndNull(&d);
            }
        }
        //release (if not consumed)
        if(obj != NULL){
            ScnApiDX12_vertexbuff_free(obj);
            obj = NULL;
        }
    }
    return r;
}

void ScnApiDX12_vertexbuff_free(void* pObj){
    STScnApiDX12VertexBuff* obj = (STScnApiDX12VertexBuff*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        ScnGpuBuffer_releaseAndNull(&obj->vBuff);
        ScnGpuBuffer_releaseAndNull(&obj->idxBuff);
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

ScnBOOL ScnApiDX12_vertexbuff_sync(void* pObj, const STScnGpuVertexbuffCfg* const cfg, ScnGpuBufferRef vBuff, ScnGpuBufferRef idxBuff){
    ScnBOOL r = ScnFALSE;
    STScnApiDX12VertexBuff* obj = (STScnApiDX12VertexBuff*)pObj;
    {
        obj->cfg = *cfg;
        ScnGpuBuffer_set(&obj->vBuff, vBuff);
        ScnGpuBuffer_set(&obj->idxBuff, idxBuff);
        //ToDo: implement
        r = ScnTRUE;
    }
    return r;
}

ScnBOOL ScnApiDX12_vertexbuff_activate(void* pObj){
    ScnBOOL r = ScnFALSE;
    //STScnApiDX12VertexBuff* obj = (STScnApiDX12VertexBuff*)pObj;
    {
        //ToDo: implement
        r = ScnTRUE;
    }
    return r;
}

ScnBOOL ScnApiDX12_vertexbuff_deactivate(void* pObj){
    ScnBOOL r = ScnFALSE;
    //STScnApiDX12VertexBuff* obj = (STScnApiDX12VertexBuff*)pObj;
    {
        //ToDo: implement
        r = ScnTRUE;
    }
    return r;
}

// STScnApiDX12Texture

typedef struct STScnApiDX12Texture {
    ScnContextRef           ctx;
    STScnGpuTextureCfg      cfg;
    id<MTLTexture>          tex;
    ScnGpuSamplerRef        sampler;
    STScnApiItf             itf;
} STScnApiDX12Texture;

ScnGpuTextureRef ScnApiDX12_device_allocTexture(void* pObj, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData){
    ScnGpuTextureRef r = ScnObjRef_Zero;
    STScnApiDX12Device* dev = (STScnApiDX12Device*)pObj;
    if(dev != NULL && dev->dev != nullptr && cfg != NULL){
        MTLPixelFormat fmt = MTLPixelFormatInvalid;
        switch (cfg->color) {
            case ENScnBitmapColor_ALPHA8: fmt = MTLPixelFormatA8Unorm; break;
            case ENScnBitmapColor_GRAY8: fmt = MTLPixelFormatR8Unorm; break;
            case ENScnBitmapColor_GRAYALPHA8: fmt = MTLPixelFormatRG8Unorm; break;
            case ENScnBitmapColor_RGBA8: fmt = MTLPixelFormatRGBA8Unorm; break;
            default: break;
        }
        if(fmt == MTLPixelFormatInvalid){
            SCN_PRINTF_ERROR("unsupported texture color format(%d).\n", cfg->color);
        } else if(cfg->width <= 0 && cfg->height <= 0){
            SCN_PRINTF_ERROR("invalid texture size(%d, %d).\n", cfg->width, cfg->height);
        } else {
            STScnApiDX12Texture* obj = NULL;
            MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
            id<MTLTexture> tex = nullptr;
            //
            texDesc.pixelFormat = fmt;
            texDesc.width       = cfg->width;
            texDesc.height      = cfg->height;
            //
            tex = [dev->dev newTextureWithDescriptor:texDesc];
            if(tex == nullptr){
                SCN_PRINTF_ERROR("newTextureWithDescriptor failed.\n");
            } else if(srcProps != NULL && (srcProps->size.width != cfg->width || srcProps->size.height != cfg->height || srcProps->color != cfg->color || srcProps->bytesPerLine <= 0 || srcData == NULL)){
                SCN_PRINTF_ERROR("texture and source props missmatch.\n");
            } else if(NULL == (obj = (STScnApiDX12Texture*)ScnContext_malloc(dev->ctx, sizeof(STScnApiDX12Texture), SCN_DBG_STR("STScnApiDX12Texture")))){
                SCN_PRINTF_ERROR("ScnContext_malloc(STScnApiDX12Texture) failed.\n");
            } else {
                ScnMemory_setZeroSt(*obj);
                ScnContext_set(&obj->ctx, dev->ctx);
                obj->itf            = dev->itf;
                obj->tex            = tex; [obj->tex retain];
                obj->cfg            = *cfg;
                obj->sampler        = ScnApiDX12_device_allocSampler(dev, &cfg->sampler);
                if(ScnGpuSampler_isNull(obj->sampler)){
                    SCN_PRINTF_ERROR("ScnApiDX12_device_allocTexture::ScnApiDX12_device_allocSampler failed.\n");
                } else {
                    ScnGpuTextureRef d = ScnGpuTexture_alloc(dev->ctx);
                    if(!ScnGpuTexture_isNull(d)){
                        STScnGpuTextureApiItf itf;
                        ScnMemory_setZeroSt(itf);
                        itf.free        = ScnApiDX12_texture_free;
                        itf.sync        = ScnApiDX12_texture_sync;
                        if(!ScnGpuTexture_prepare(d, &itf, obj)){
                            SCN_PRINTF_ERROR("ScnApiDX12_device_allocTexture::ScnGpuTexture_prepare failed.\n");
                        } else {
                            //apply data
                            if(srcProps != NULL && srcData != NULL){
                                MTLRegion region = { { 0, 0, 0 }, {cfg->width, cfg->height, 1} };
                                [tex replaceRegion:region mipmapLevel:0 withBytes:srcData bytesPerRow:srcProps->bytesPerLine];
                            }
                            //
                            ScnGpuTexture_set(&r, d);
                            obj = NULL; //consume
                        }
                        ScnGpuTexture_releaseAndNull(&d);
                    }
                }
            }
            //
            if(texDesc != nullptr){ [texDesc release]; texDesc = nullptr; }
            if(tex != nullptr){ [tex release]; tex = nullptr; }
            if(obj != NULL){
                ScnApiDX12_texture_free(obj);
                obj = NULL;
            }
        }
    }
    return r;
}

void ScnApiDX12_texture_free(void* pObj){
    STScnApiDX12Texture* obj = (STScnApiDX12Texture*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        if(obj->tex != nullptr){
            [obj->tex release];
            obj->tex = nullptr;
        }
        ScnGpuSampler_releaseAndNull(&obj->sampler);
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

ScnBOOL ScnApiDX12_texture_sync(void* pObj, const STScnGpuTextureCfg* const cfg, const STScnBitmapProps* const srcProps, const void* srcData, const STScnGpuTextureChanges* const changes){
    ScnBOOL r = ScnFALSE;
    STScnApiDX12Texture* obj = (STScnApiDX12Texture*)pObj;
    if(!(obj != NULL && obj->tex != NULL && srcProps != NULL && srcData != NULL && changes != NULL)){
        //missing params or objects
        return r;
    }
    if(!(obj->cfg.width == cfg->width && obj->cfg.height == cfg->height && obj->cfg.color == cfg->color)){
        //change of size of color is not supported
        return r;
    }
    //sync
    {
        ScnUI32 pxUpdated = 0, lnsUpdated = 0;
        if(changes->all){
            //update the texture
            MTLRegion region = { { 0, 0, 0 }, {obj->cfg.width, obj->cfg.height, 1} };
            [obj->tex replaceRegion:region mipmapLevel:0 withBytes:srcData bytesPerRow:srcProps->bytesPerLine];
            pxUpdated   += (obj->cfg.width * obj->cfg.height);
            lnsUpdated  += obj->cfg.height;
            r = ScnTRUE;
        } else if(changes->rngsUse == 0){
            //nothing to sync
            r = ScnTRUE;
        } else if(changes->rngs != 0){
#           ifdef SCN_ASSERTS_ACTIVATED
            ScnUI32 prevRngAfterEnd = 0;
#           endif
            const STScnRangeU* rng = changes->rngs;
            const STScnRangeU* rngAfterEnd = rng + changes->rngsUse;
            r = ScnTRUE;
            while(rng < rngAfterEnd){
                SCN_ASSERT(prevRngAfterEnd <= rng->start + rng->size) //rngs should be ordered and non-overlapping
                if(rng->start > obj->cfg.height || (rng->start + rng->size) > obj->cfg.height){
                    //out of range
                    r = ScnFALSE;
                    break;
                }
                //update the texture area
                {
                    MTLRegion region = { { 0, rng->start, 0 }, {obj->cfg.width, rng->size, 1} };
                    const void* srcRow = ((ScnBYTE*)srcData) + (rng->start * srcProps->bytesPerLine);
                    [obj->tex replaceRegion:region mipmapLevel:0 withBytes:srcRow bytesPerRow:srcProps->bytesPerLine];
                    pxUpdated   += (obj->cfg.width * rng->size);
                    lnsUpdated  += rng->size;
                }
#               ifdef SCN_ASSERTS_ACTIVATED
                prevRngAfterEnd = rng->start + rng->size;
#               endif
                ++rng;
            }
            r = ScnTRUE;
        }
        SCN_PRINTF_VERB("%2.f%% %u of %u lines synced at texture (%.1f Kpixs, %.1f KBs).\n", (float)lnsUpdated * 100.f / (float)obj->cfg.height, lnsUpdated, obj->cfg.height, (ScnFLOAT)pxUpdated / 1024.f, (ScnFLOAT)lnsUpdated * (ScnFLOAT)srcProps->bytesPerLine / 1024.f);
    }
    return r;
}

//STScnApiDX12RenderStates

#define ScnApiDX12RenderStates_ENScnVertexTypeFromVBuffCfg(VBUFF_CFG)  (VBUFF_CFG->texCoords[ENScnGpuTextureIdx_2].amm > 0 ? ENScnVertexType_2DTex3 : VBUFF_CFG->texCoords[ENScnGpuTextureIdx_1].amm > 0 ? ENScnVertexType_2DTex2 : VBUFF_CFG->texCoords[ENScnGpuTextureIdx_0].amm > 0 ? ENScnVertexType_2DTex : ENScnVertexType_2DColor)

typedef struct STScnApiDX12RenderStates {
    MTLPixelFormat              color;
    id<MTLRenderPipelineState>  states[ENScnVertexType_Count]; //shader and fragment for this framebuffer
} STScnApiDX12RenderStates;

void    STScnApiDX12RenderStates_init(STScnApiDX12RenderStates* obj);
void    STScnApiDX12RenderStates_destroy(STScnApiDX12RenderStates* obj);
ScnBOOL STScnApiDX12RenderStates_load(STScnApiDX12RenderStates* obj, STScnApiDX12Device* dev, MTLPixelFormat color);

//STScnApiDX12FramebuffView

typedef struct STScnApiDX12FramebuffView {
    ScnContextRef           ctx;
    MTKView*                mtkView;
    STScnSize2DU            size;
    STScnGpuFramebuffProps  props;
    STScnApiItf             itf;
    STScnApiDX12RenderStates rndrShaders; //shaders
    //cur (state while sending commands)
    struct {
        //verts
        struct {
            ENScnVertexType type;
            STScnApiDX12Buffer* buff;
            STScnApiDX12Buffer* idxs;
        } verts;
    } cur;
} STScnApiDX12FramebuffView;

ScnGpuFramebuffRef ScnApiDX12_device_allocFramebuffFromOSView(void* pObj, void* pMtkView){
    ScnGpuFramebuffRef r = ScnObjRef_Zero;
    STScnApiDX12Device* dev = (STScnApiDX12Device*)pObj;
    MTKView* mtkView = (MTKView*)pMtkView;
    if(dev != NULL && dev->dev != NULL && mtkView != nullptr){
        STScnApiDX12FramebuffView* obj = NULL;
        STScnApiDX12RenderStates rndrShaders;
        STScnApiDX12RenderStates_init(&rndrShaders);
        if(!STScnApiDX12RenderStates_load(&rndrShaders, dev, mtkView.colorPixelFormat)){
            SCN_PRINTF_ERROR("STScnApiDX12RenderStates_load failed.\n");
        } else if(NULL == (obj = (STScnApiDX12FramebuffView*)ScnContext_malloc(dev->ctx, sizeof(STScnApiDX12FramebuffView), SCN_DBG_STR("STScnApiDX12FramebuffView")))){
            SCN_PRINTF_ERROR("ScnContext_malloc(STScnApiDX12FramebuffView) failed.\n");
            STScnApiDX12RenderStates_destroy(&rndrShaders);
        } else {
            CGSize viewSz = mtkView.drawableSize;
            ScnMemory_setZeroSt(*obj);
            ScnContext_set(&obj->ctx, dev->ctx);
            obj->itf            = dev->itf;
            obj->mtkView        = mtkView; [obj->mtkView retain];
            obj->rndrShaders   = rndrShaders;
            {
                //size
                obj->size.width             = viewSz.width;
                obj->size.height            = viewSz.height;
                //viewport
                obj->props.viewport.x       = 0;
                obj->props.viewport.y       = 0;
                obj->props.viewport.width   = obj->size.width;
                obj->props.viewport.height  = obj->size.height;
                //ortho2d
                obj->props.ortho.x.min      = 0.f;
                obj->props.ortho.x.max      = obj->size.width;
                obj->props.ortho.y.min      = 0.f;
                obj->props.ortho.y.max      = obj->size.height;
            }
            //
            ScnGpuFramebuffRef d = ScnGpuFramebuff_alloc(dev->ctx);
            if(!ScnGpuFramebuff_isNull(d)){
                STScnGpuFramebuffApiItf itf;
                ScnMemory_setZeroSt(itf);
                itf.free        = ScnApiDX12_framebuff_view_free;
                itf.getSize     = ScnApiDX12_framebuff_view_getSize;
                itf.syncSize    = ScnApiDX12_framebuff_view_syncSize;
                itf.getProps    = ScnApiDX12_framebuff_view_getProps;
                itf.setProps    = ScnApiDX12_framebuff_view_setProps;
                if(!ScnGpuFramebuff_prepare(d, &itf, obj)){
                    SCN_PRINTF_ERROR("ScnApiDX12_device_allocFramebuffFromOSView::ScnGpuFramebuff_prepare failed.\n");
                } else {
                    //configure view
                    mtkView.device = dev->dev;
                    //
                    ScnGpuFramebuff_set(&r, d);
                    obj = NULL; //consume
                }
                ScnGpuFramebuff_releaseAndNull(&d);
            }
        }
        //release (if not consumed)
        if(obj != NULL){
            ScnApiDX12_framebuff_view_free(obj);
            obj = NULL;
        }
    }
    return r;
}

//frameBuffer (view)

void ScnApiDX12_framebuff_view_free(void* pObj){
    STScnApiDX12FramebuffView* obj = (STScnApiDX12FramebuffView*)pObj;
    ScnContextRef ctx = obj->ctx;
    {
        //
        STScnApiDX12RenderStates_destroy(&obj->rndrShaders);
        //
        if(obj->mtkView != nullptr){
            [obj->mtkView release];
            obj->mtkView = nullptr;
        }
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

STScnSize2DU ScnApiDX12_framebuff_view_getSize(void* pObj){
    STScnApiDX12FramebuffView* obj = (STScnApiDX12FramebuffView*)pObj;
    return obj->size;
}

ScnBOOL ScnApiDX12_framebuff_view_syncSize(void* pObj, const STScnSize2DU size){
    ScnBOOL r = ScnFALSE;
    STScnApiDX12FramebuffView* obj = (STScnApiDX12FramebuffView*)pObj;
    if(obj->mtkView != nullptr){
        //const CGSize sz = obj->mtkView.drawableSize;
        //obj->size.width = sz.width;
        //obj->size.height = sz.height;
        obj->size = size;
        r = ScnTRUE;
    }
    return r;
}

STScnGpuFramebuffProps ScnApiDX12_framebuff_view_getProps(void* pObj){
    STScnApiDX12FramebuffView* obj = (STScnApiDX12FramebuffView*)pObj;
    return obj->props;
}

ScnBOOL ScnApiDX12_framebuff_view_setProps(void* pObj, const STScnGpuFramebuffProps* const props){
    ScnBOOL r = ScnFALSE;
    STScnApiDX12FramebuffView* obj = (STScnApiDX12FramebuffView*)pObj;
    if(obj->mtkView != nullptr && props != NULL){
        obj->props = *props;
        r = ScnTRUE;
    }
    return r;
}

// STScnApiDX12RenderStates

void STScnApiDX12RenderStates_init(STScnApiDX12RenderStates* obj){
    ScnMemory_setZeroSt(*obj);
}

void STScnApiDX12RenderStates_destroy(STScnApiDX12RenderStates* obj){
    //states
    {
        id<MTLRenderPipelineState>* s = &obj->states[0];
        id<MTLRenderPipelineState>* sAfterEnd = s + (sizeof(obj->states) / sizeof(obj->states[0]));
        while(s < sAfterEnd){
            if(*s != nullptr){
                [*s release];
                *s  = nullptr;
            }
            ++s;
        }
    }
}

//

ScnBOOL STScnApiDX12RenderStates_load(STScnApiDX12RenderStates* obj, STScnApiDX12Device* dev, MTLPixelFormat color){
    ScnBOOL r = ScnTRUE;
    obj->color = color;
    {
        id<MTLFunction> vertexFunc = nullptr;
        id<MTLFunction> fragmtFunc = nullptr;
        ScnUI32 i; for(i = 0; i < (sizeof(obj->states) / sizeof(obj->states[0])); ++i){
            const char* vertexFuncName = NULL;
            const char* fragmtFuncName = NULL;
            switch (i) {
                case ENScnVertexType_2DColor: vertexFuncName = "ixtliVertexShader"; fragmtFuncName = "ixtliFragmentShader"; break;
                case ENScnVertexType_2DTex:   vertexFuncName = "ixtliVertexShaderTex"; fragmtFuncName = "ixtliFragmentShaderTex"; break;
                case ENScnVertexType_2DTex2:  vertexFuncName = "ixtliVertexShaderTex2"; fragmtFuncName = "ixtliFragmentShaderTex2"; break;
                case ENScnVertexType_2DTex3:  vertexFuncName = "ixtliVertexShaderTex3"; fragmtFuncName = "ixtliFragmentShaderTex3"; break;
                default: break;
            }
            if(vertexFuncName == NULL){
                SCN_PRINTF_ERROR("STScnApiDX12RenderStates_load unimplemented function name.\n");
                r = ScnFALSE;
                break;
            }
            if(fragmtFuncName == NULL){
                SCN_PRINTF_ERROR("STScnApiDX12RenderStates_load unimplemented function name.\n");
                r = ScnFALSE;
                break;
            }
            //
            if(vertexFunc != nullptr){ [vertexFunc release]; vertexFunc = nullptr; }
            if(fragmtFunc != nullptr){ [fragmtFunc release]; fragmtFunc = nullptr; }
            //
            vertexFunc = [dev->lib newFunctionWithName:[NSString stringWithUTF8String:vertexFuncName]];
            if(vertexFunc == nullptr){
                SCN_PRINTF_ERROR("STScnApiDX12RenderStates_load newFunctionWithName('%s') failed.\n", vertexFuncName);
                r = ScnFALSE;
                break;
            }
            fragmtFunc = [dev->lib newFunctionWithName:[NSString stringWithUTF8String:fragmtFuncName]];
            if(fragmtFunc == nullptr){
                SCN_PRINTF_ERROR("STScnApiDX12RenderStates_load newFunctionWithName('%s') failed.\n", fragmtFuncName);
                r = ScnFALSE;
                break;
            }
            //create state
            {
                NSError *error;
                MTLRenderPipelineDescriptor* rndrPipeDesc = [[MTLRenderPipelineDescriptor alloc] init];
                rndrPipeDesc.label = @"Ixtla-render default (fixed) render pipeline.";
                rndrPipeDesc.vertexFunction = vertexFunc; [vertexFunc retain];
                rndrPipeDesc.fragmentFunction = fragmtFunc; [fragmtFunc retain];
                rndrPipeDesc.colorAttachments[0].pixelFormat = color;
                rndrPipeDesc.colorAttachments[0].blendingEnabled = YES;
                rndrPipeDesc.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
                rndrPipeDesc.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
                rndrPipeDesc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
                rndrPipeDesc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
                rndrPipeDesc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
                rndrPipeDesc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
                if(nullptr == (obj->states[i] = [dev->dev newRenderPipelineStateWithDescriptor:rndrPipeDesc error:&error])){
                    SCN_PRINTF_ERROR("STScnApiDX12RenderStates_load::newRenderPipelineStateWithDescriptor failed: '%s'.\n", (error == nullptr ? "nullptr" : [[error description] UTF8String]));
                    r = ScnFALSE;
                    break;
                }
            }
        }
        if(vertexFunc != nullptr){ [vertexFunc release]; vertexFunc = nullptr; }
        if(fragmtFunc != nullptr){ [fragmtFunc release]; fragmtFunc = nullptr; }
    }
    return r;
}

// STScnApiDX12RenderJobState

typedef struct STScnApiDX12RenderJobState {
    STScnApiDX12FramebuffView* fb;
    MTLRenderPassDescriptor*    rndrDesc; //per active framebuff
    id<MTLRenderCommandEncoder> rndrEnc; //per active framebuff
} STScnApiDX12RenderJobState;

void ScnApiDX12RenderJobState_init(STScnApiDX12RenderJobState* obj);
void ScnApiDX12RenderJobState_destroy(STScnApiDX12RenderJobState* obj);
//
void ScnApiDX12RenderJobState_reset(STScnApiDX12RenderJobState* obj);

// STScnApiDX12RenderJob

typedef struct STScnApiDX12RenderJob {
    ScnContextRef           ctx;
    STScnApiDX12Device*    dev;
    //bPropsScns
    struct {
        ScnGpuBufferRef     ref;
        STScnApiDX12Buffer* obj;
    } bPropsScns;
    //bPropsMdls
    struct {
        ScnGpuBufferRef     ref;
        STScnApiDX12Buffer* obj;
    } bPropsMdls;
    //
    id<MTLCommandBuffer>    cmdsBuff;
    //
    STScnApiDX12RenderJobState state;
} STScnApiDX12RenderJob;

ScnGpuRenderJobRef ScnApiDX12_device_allocRenderJob(void* pObj){
    ScnGpuRenderJobRef r = ScnGpuRenderJobRef_Zero;
    STScnApiDX12Device* dev = (STScnApiDX12Device*)pObj;
    if(dev != NULL && dev->dev != NULL && dev->cmdQueue != nullptr){
        STScnApiDX12RenderJob* obj = (STScnApiDX12RenderJob*)ScnContext_malloc(dev->ctx, sizeof(STScnApiDX12RenderJob), SCN_DBG_STR("STScnApiDX12RenderJob"));
        if(obj == NULL){
            SCN_PRINTF_ERROR("ScnContext_malloc(STScnApiDX12RenderJob) failed.\n");
        } else {
            ScnMemory_setZeroSt(*obj);
            ScnContext_set(&obj->ctx, dev->ctx);
            ScnApiDX12RenderJobState_init(&obj->state);
            obj->dev        = dev;  //retain?
            //
            ScnGpuRenderJobRef d = ScnGpuRenderJob_alloc(dev->ctx);
            if(!ScnGpuRenderJob_isNull(d)){
                //
                STScnGpuRenderJobApiItf itf;
                ScnMemory_setZeroSt(itf);
                itf.free        = ScnApiDX12_renderJob_free;
                itf.getState    = ScnApiDX12_renderJob_getState;
                itf.buildBegin  = ScnApiDX12_renderJob_buildBegin;
                itf.buildAddCmds = ScnApiDX12_renderJob_buildAddCmds;
                itf.buildEndAndEnqueue = ScnApiDX12_renderJob_buildEndAndEnqueue;
                //
                if(!ScnGpuRenderJob_prepare(d, &itf, obj)){
                    SCN_PRINTF_ERROR("ScnApiDX12_device_allocRenderJob::ScnGpuRenderJob_prepare failed.\n");
                } else {
                    ScnGpuRenderJob_set(&r, d);
                    obj = NULL; //consume
                }
                ScnGpuRenderJob_releaseAndNull(&d);
            }
        }
        //release (if not consumed)
        if(obj != NULL){
            ScnApiDX12_renderJob_free(obj);
            obj = NULL;
        }
    }
    return r;
}


//render job

void ScnApiDX12_renderJob_free(void* data){
    STScnApiDX12RenderJob* obj = (STScnApiDX12RenderJob*)data;
    ScnContextRef ctx = obj->ctx;
    {
        if(obj->cmdsBuff != nullptr){
#           ifdef SCN_ASSERTS_ACTIVATED
            const MTLCommandBufferStatus status = [obj->cmdsBuff status];
            SCN_ASSERT(status == MTLCommandBufferStatusNotEnqueued || status == MTLCommandBufferStatusCompleted || status == MTLCommandBufferStatusError) //should not be active
#           endif
            [obj->cmdsBuff release];
            obj->cmdsBuff = nullptr;
        }
        //bPropsScns
        {
            ScnGpuBuffer_releaseAndNull(&obj->bPropsScns.ref);
            obj->bPropsScns.obj = NULL;
        }
        //bPropsMdls
        {
            ScnGpuBuffer_releaseAndNull(&obj->bPropsMdls.ref);
            obj->bPropsMdls.obj = NULL;
        }
        ScnApiDX12RenderJobState_destroy(&obj->state);
        ScnContext_null(&obj->ctx);
    }
    ScnContext_mfree(ctx, obj);
    ScnContext_releaseAndNull(&ctx);
}

ENScnGpuRenderJobState ScnApiDX12_renderJob_getState(void* data){
    ENScnGpuRenderJobState r = ENScnGpuRenderJobState_Unknown;
    STScnApiDX12RenderJob* obj = (STScnApiDX12RenderJob*)data;
    if(obj->cmdsBuff != nullptr){
        switch ([obj->cmdsBuff status]) {
            case MTLCommandBufferStatusNotEnqueued: ; break;
            case MTLCommandBufferStatusEnqueued: r = ENScnGpuRenderJobState_Enqueued; break;
            case MTLCommandBufferStatusCommitted: r = ENScnGpuRenderJobState_Enqueued; break;
            case MTLCommandBufferStatusScheduled: r = ENScnGpuRenderJobState_Enqueued; break;
            case MTLCommandBufferStatusCompleted: r = ENScnGpuRenderJobState_Completed; break;
            case MTLCommandBufferStatusError: r = ENScnGpuRenderJobState_Error; break;
            default: break;
        }
    }
    return r;
}

ScnBOOL ScnApiDX12_renderJob_buildBegin(void* data, ScnGpuBufferRef pBuffPropsScns, ScnGpuBufferRef pBuffPropsMdls){
    ScnBOOL r = ScnFALSE;
    STScnApiDX12RenderJob* obj = (STScnApiDX12RenderJob*)data;
    //
    STScnApiDX12Buffer* bPropsScns = NULL;
    STScnApiDX12Buffer* bPropsMdls = NULL;
    //
    if(obj->dev == NULL || obj->dev->dev == nullptr || obj->dev->cmdQueue == nullptr){
        SCN_PRINTF_ERROR("ScnApiDX12_renderJob_buildBegin::dev-or-cmdQueue is NULL.\n");
        return ScnFALSE;
    }
    if(ScnGpuBuffer_isNull(pBuffPropsScns) || ScnGpuBuffer_isNull(pBuffPropsMdls)){
        //SCN_PRINTF_ERROR("ScnApiDX12_renderJob_buildBegin::pBuffPropsScns-or-pBuffPropsMdls is NULL.\n");
        return ScnFALSE;
    }
    bPropsScns = (STScnApiDX12Buffer*)ScnGpuBuffer_getApiItfParam(pBuffPropsScns);
    bPropsMdls = (STScnApiDX12Buffer*)ScnGpuBuffer_getApiItfParam(pBuffPropsMdls);
    if(bPropsScns == NULL || bPropsScns->buff == nullptr){
        SCN_PRINTF_ERROR("ScnApiDX12_renderJob_buildBegin::bPropsScns is NULL.\n");
        return ScnFALSE;
    } else if(bPropsMdls == NULL || bPropsMdls->buff == nullptr){
        SCN_PRINTF_ERROR("ScnApiDX12_renderJob_buildBegin::bPropsMdls is NULL.\n");
        return ScnFALSE;
    }
    if(obj->cmdsBuff != nullptr){
        const MTLCommandBufferStatus status = [obj->cmdsBuff status];
        if(status != MTLCommandBufferStatusNotEnqueued && status != MTLCommandBufferStatusCompleted && status != MTLCommandBufferStatusError){
            //job currentyl in progress
            SCN_PRINTF_ERROR("ScnApiDX12_renderJob_buildBegin::[obj->cmdsBuff status] is active.\n");
            return ScnFALSE;
        }
    }
    //begin
    {
        if(obj->cmdsBuff != nullptr){
            [obj->cmdsBuff release];
            obj->cmdsBuff = nullptr;
        }
        obj->cmdsBuff = [obj->dev->cmdQueue commandBuffer];
        if(obj->cmdsBuff != nullptr){
            obj->cmdsBuff.label = @"Ixtli-cmd-buff";
            [obj->cmdsBuff retain];
            {
                ScnGpuBuffer_set(&obj->bPropsScns.ref, pBuffPropsScns);
                obj->bPropsScns.obj = bPropsScns;
            }
            {
                ScnGpuBuffer_set(&obj->bPropsMdls.ref, pBuffPropsMdls);
                obj->bPropsMdls.obj = bPropsMdls;
            }
            ScnApiDX12RenderJobState_reset(&obj->state);
            //
            r = ScnTRUE;
        }
    }
    //
    return r;
    
}

ScnBOOL ScnApiDX12_renderJob_buildAddCmds(void* data, const struct STScnRenderCmd* const cmds, const ScnUI32 cmdsSz){
    ScnBOOL r = ScnFALSE;
    //
    STScnApiDX12RenderJob* obj = (STScnApiDX12RenderJob*)data;
    //
    if(obj->cmdsBuff == nullptr){
        return ScnFALSE;
    } else if(obj->bPropsScns.obj == NULL || obj->bPropsScns.obj->buff == nullptr){
        return ScnFALSE;
    } else if(obj->bPropsMdls.obj == NULL || obj->bPropsMdls.obj->buff == nullptr){
        return ScnFALSE;
    }
    //
    STScnApiDX12RenderJobState* state = &obj->state;
    const STScnRenderCmd* c = cmds;
    const STScnRenderCmd* cAfterEnd = cmds + cmdsSz;
    //
    r = ScnTRUE;
    //
    while(r && c < cAfterEnd){
        switch (c->cmdId) {
            case ENScnRenderCmd_None:
                //nop
                break;
                //framebuffers
            case ENScnRenderCmd_ActivateFramebuff:
                if(!ScnFramebuff_isNull(c->activateFramebuff.ref)){
                    ScnGpuFramebuffRef gpuFb = ScnFramebuff_getCurrentRenderSlot(c->activateFramebuff.ref);
                    if(ScnGpuFramebuff_isNull(gpuFb)){
                        SCN_PRINTF_ERROR("ENScnRenderCmd_ActivateFramebuff::ScnGpuFramebuff_isNull.\n");
                        r = ScnFALSE;
                    } else {
                        state->fb = (STScnApiDX12FramebuffView*)ScnGpuFramebuff_getApiItfParam(gpuFb);
                        if(state->fb == NULL || state->fb->mtkView == nullptr || state->fb->rndrShaders.states[state->fb->cur.verts.type] == nullptr){
                            SCN_PRINTF_ERROR("ENScnRenderCmd_ActivateFramebuff::fb == NULL || fb->mtkView == nullptr || fb->renderState == nullptr.\n");
                            r = ScnFALSE;
                            break;
                        }
                        //ToDo: define how to reset state
                        //ScnMemory_setZeroSt(fb->cur);
                        //
                        MTLRenderPassDescriptor* rndrDesc = state->fb->mtkView.currentRenderPassDescriptor;
                        SCN_ASSERT(obj->cmdsBuff != nullptr)
                        if(rndrDesc == nullptr){
                            SCN_PRINTF_ERROR("ENScnRenderCmd_ActivateFramebuff::renderPassDescriptor == nullptr || commandBuffer == nullptr.\n");
                            r = ScnFALSE;
                        } else {
                            id <MTLRenderCommandEncoder> rndrEnc = [obj->cmdsBuff renderCommandEncoderWithDescriptor:rndrDesc];
                            if(rndrEnc == nullptr){
                                SCN_PRINTF_ERROR("ENScnRenderCmd_ActivateFramebuff::renderCommandEncoderWithDescriptor failed.\n");
                                r = ScnFALSE;
                            } else {
                                if(state->rndrDesc != nullptr){ [state->rndrDesc release]; state->rndrDesc = nullptr; }
                                if(state->rndrEnc != nullptr){ [state->rndrEnc release]; state->rndrEnc = nullptr; }
                                //
                                state->rndrDesc = rndrDesc; [rndrDesc retain];
                                state->rndrEnc = rndrEnc; [rndrEnc retain];
                                state->rndrEnc.label = @"ixtli-render-cmd-enc";
                                //
                                [state->rndrEnc setRenderPipelineState:state->fb->rndrShaders.states[state->fb->cur.verts.type]];
                                //apply viewport
                                {
                                    MTLViewport viewPort;
                                    viewPort.originX = (double)c->activateFramebuff.viewport.x;
                                    viewPort.originY = (double)c->activateFramebuff.viewport.y;
                                    viewPort.width = (double)c->activateFramebuff.viewport.width;
                                    viewPort.height = (double)c->activateFramebuff.viewport.height;
                                    viewPort.znear = 0.0;
                                    viewPort.zfar = 1.0;
                                    [state->rndrEnc setViewport:viewPort];
                                    SCN_PRINTF_VERB("setViewport(%u, %u)-(+%u, +%u).\n", fb->viewport.x, fb->viewport.y, fb->viewport.width, fb->viewport.height);
                                }
                                //fb props
                                [state->rndrEnc setVertexBuffer:obj->bPropsScns.obj->buff offset:c->activateFramebuff.offset atIndex:0];
                                SCN_PRINTF_VERB("setVertexBuffer(idx0, %u offset).\n", c->activateFramebuff.offset);
                                //mdl props
                                [state->rndrEnc setVertexBuffer:obj->bPropsMdls.obj->buff offset:0 atIndex:1];
                                SCN_PRINTF_VERB("setVertexBuffer(idx1, 0 offset).\n");
                            }
                        }
                    }
                }
                break;
            case ENScnRenderCmd_SetFramebuffProps:
                if(state->rndrEnc != nullptr){
                    //apply viewport
                    {
                        MTLViewport viewPort;
                        viewPort.originX = (double)c->setFramebuffProps.viewport.x;
                        viewPort.originY = (double)c->setFramebuffProps.viewport.y;
                        viewPort.width = (double)c->setFramebuffProps.viewport.width;
                        viewPort.height = (double)c->setFramebuffProps.viewport.height;
                        viewPort.znear = 0.0;
                        viewPort.zfar = 1.0;
                        [state->rndrEnc setViewport:viewPort];
                        SCN_PRINTF_VERB("setViewport(%u, %u)-(+%u, +%u).\n", fb->viewport.x, fb->viewport.y, fb->viewport.width, fb->viewport.height);
                    }
                    //fb props
                    [state->rndrEnc setVertexBufferOffset:c->setFramebuffProps.offset atIndex:0];
                }
                break;
                //models
            case ENScnRenderCmd_SetTransformOffset: //sets the positions of the 'STScnGpuModelProps2d' to be applied for the drawing cmds
                if(state->rndrEnc == nullptr){
                    SCN_PRINTF_ERROR("ENScnRenderCmd_SetTransformOffset::rndrEnc is nullptr.\n");
                    r = ScnFALSE;
                } else {
                    [state->rndrEnc setVertexBufferOffset:c->setTransformOffset.offset atIndex:1];
                    SCN_PRINTF_VERB("setVertexBufferOffset(idx1, %u offset).\n", c->setTransformOffset.offset);
                }
                break;
            case ENScnRenderCmd_SetVertexBuff:  //activates the vertex buffer
                if(ScnVertexbuff_isNull(c->setVertexBuff.ref)){
                    SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::ScnVertexbuff_isNull.\n");
                    [state->rndrEnc setVertexBuffer:nullptr offset:0 atIndex:2];
                } else {
                    ScnGpuVertexbuffRef vbuffRef = ScnVertexbuff_getCurrentRenderSlot(c->setVertexBuff.ref);
                    if(ScnGpuVertexbuff_isNull(vbuffRef)){
                        SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::ScnGpuVertexbuff_isNull.\n");
                        r = ScnFALSE;
                    } else {
                        STScnApiDX12VertexBuff* vbuff = (STScnApiDX12VertexBuff*)ScnGpuVertexBuff_getApiItfParam(vbuffRef);
                        if(vbuff == NULL || ScnGpuBuffer_isNull(vbuff->vBuff)){
                            SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::ScnGpuBuffer_isNull.\n");
                            r = ScnFALSE;
                        } else {
                            STScnApiDX12Buffer* buff = (STScnApiDX12Buffer*)ScnGpuBuffer_getApiItfParam(vbuff->vBuff);
                            STScnApiDX12Buffer* idxs = ScnGpuBuffer_isNull(vbuff->idxBuff) ? NULL : (STScnApiDX12Buffer*)ScnGpuBuffer_getApiItfParam(vbuff->idxBuff);
                            if(buff == NULL){
                                SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::buff == NULL.\n");
                                r = ScnFALSE;
                            } else {
                                const ENScnVertexType vertexType = STScnGpuVertexbuffCfg_2_ENScnVertexType(&vbuff->cfg);
                                if(state->fb->rndrShaders.states[state->fb->cur.verts.type] == nullptr){
                                    SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::fb->rndrShaders.states[fb->curVertexType] == nullptr.\n");
                                    r = ScnFALSE;
                                } else {
                                    state->fb->cur.verts.type  = vertexType;
                                    state->fb->cur.verts.buff  = buff;
                                    state->fb->cur.verts.idxs  = idxs;
                                    //MTLRenderStageVertex   = (1UL << 0),
                                    //MTLRenderStageFragment = (1UL << 1),
                                    //if(c->setVertexBuff.isFirstUse){
                                    //    [state->rndrEnc useResource:buff->buff usage:MTLResourceUsageRead stages:MTLRenderStageVertex];
                                    //}
                                    [state->rndrEnc setRenderPipelineState:state->fb->rndrShaders.states[state->fb->cur.verts.type]];
                                    [state->rndrEnc setVertexBuffer:buff->buff offset:0 atIndex:2];
                                    SCN_PRINTF_VERB("setVertexBuffer(idx2, 0 offset).\n");
                                }
                            }
                        }
                    }
                }
                break;
           case ENScnRenderCmd_SetTexture:     //activates the texture in a specific slot-index
                if(ScnTexture_isNull(c->setTexture.ref)){
                    [state->rndrEnc setFragmentTexture:nullptr atIndex:c->setTexture.index];
                    [state->rndrEnc setFragmentSamplerState:nullptr atIndex:c->setTexture.index];
                } else {
                    ScnGpuTextureRef texRef = ScnTexture_getCurrentRenderSlot(c->setTexture.ref);
                    if(ScnGpuTexture_isNull(texRef)){
                        SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::ScnGpuTexture_isNull.\n");
                        r = ScnFALSE;
                    } else {
                        STScnApiDX12Texture* tex = (STScnApiDX12Texture*)ScnGpuTexture_getApiItfParam(texRef);
                        if(tex == NULL || tex->tex == nullptr || ScnGpuSampler_isNull(tex->sampler)){
                            SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::tex->tex is NULL.\n");
                            r = ScnFALSE;
                        } else {
                            STScnApiDX12Sampler* smplr = (STScnApiDX12Sampler*)ScnGpuSampler_getApiItfParam(tex->sampler);
                            if(smplr->smplr == nullptr){
                                SCN_PRINTF_ERROR("ENScnRenderCmd_SetVertexBuff::smplr->smplr is NULL.\n");
                                r = ScnFALSE;
                            } else {
                                //if(c->setTexture.isFirstUse){
                                //    [state->rndrEnc useResource:tex->tex usage:MTLResourceUsageRead stages:MTLRenderStageFragment];
                                //}
                                [state->rndrEnc setFragmentTexture:tex->tex atIndex:c->setTexture.index];
                                [state->rndrEnc setFragmentSamplerState:smplr->smplr atIndex:c->setTexture.index];
                            }
                        }
                    }
                }
                break;
                //modes
                //case ENScnRenderCmd_MaskModePush:   //pushes drawing-mask mode, where only the alpha value is affected
                //    break;
                //case ENScnRenderCmd_MaskModePop:    //pop
                //    break;
                //drawing
            case ENScnRenderCmd_DrawVerts:      //draws something using the vertices
                switch (c->drawVerts.shape) {
                    case ENScnRenderShape_Compute:
                        //nop
                        break;
                        //
                    case ENScnRenderShape_Texture:     //same as 'ENScnRenderShape_TriangStrip' with possible bitblit-optimization if matrix has no rotation.
                        if(state->rndrEnc == NULL){
                            //rintf("ERROR, ENScnRenderShape_Texture::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:c->drawVerts.iFirst vertexCount:c->drawVerts.count];
                            SCN_PRINTF_VERB("drawPrimitives(MTLPrimitiveTypeTriangleStrip: %u, +%u).\n", c->drawVerts.iFirst, c->drawVerts.count);
                        }
                        break;
                    case ENScnRenderShape_TriangStrip: //triangles-strip, most common shape
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_TriangStrip::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:c->drawVerts.iFirst vertexCount:c->drawVerts.count];
                            SCN_PRINTF_VERB("drawPrimitives(MTLPrimitiveTypeTriangleStrip: %u, +%u).\n", c->drawVerts.iFirst, c->drawVerts.count);
                        }
                        break;
                    //case ENScnRenderShape_TriangFan:   //triangles-fan
                    //    break;
                        //
                    case ENScnRenderShape_LineStrip:   //lines-strip
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_LineStrip::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawPrimitives:MTLPrimitiveTypeLineStrip vertexStart:c->drawVerts.iFirst vertexCount:c->drawVerts.count];
                            SCN_PRINTF_VERB("drawPrimitives(MTLPrimitiveTypeLineStrip: %u, +%u).\n", c->drawVerts.iFirst, c->drawVerts.count);
                        }
                        break;
                    //case ENScnRenderShape_LineLoop:    //lines-loop
                    //    break;
                    case ENScnRenderShape_Lines:       //lines
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_Lines::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawPrimitives:MTLPrimitiveTypeLine vertexStart:c->drawVerts.iFirst vertexCount:c->drawVerts.count];
                            SCN_PRINTF_VERB("drawPrimitives(MTLPrimitiveTypeLine: %u, +%u).\n", c->drawVerts.iFirst, c->drawVerts.count);
                        }
                        break;
                    case ENScnRenderShape_Points:      //points
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_Points::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawPrimitives:MTLPrimitiveTypePoint vertexStart:c->drawVerts.iFirst vertexCount:c->drawVerts.count];
                            SCN_PRINTF_VERB("drawPrimitives(MTLPrimitiveTypePoint: %u, +%u).\n", c->drawVerts.iFirst, c->drawVerts.count);
                        }
                        break;
                    default:
                        SCN_ASSERT(ScnFALSE); //missing implementation
                        break;
                }
                break;
            case ENScnRenderCmd_DrawIndexes:    //draws something using the vertices indexes
                if(state->fb == NULL || state->fb->cur.verts.idxs == NULL || state->fb->cur.verts.idxs->buff == nullptr){
                    SCN_PRINTF_ERROR("ENScnRenderCmd_DrawIndexes without active framebuffer or index-buffer.\n");
                    r = ScnFALSE;
                    break;
                }
                switch (c->drawIndexes.shape) {
                    case ENScnRenderShape_Compute:
                        //nop
                        break;
                        //
                    case ENScnRenderShape_Texture:     //same as 'ENScnRenderShape_TriangStrip' with possible bitblit-optimization if matrix has no rotation.
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_Texture::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawIndexedPrimitives:MTLPrimitiveTypeTriangleStrip indexCount:c->drawIndexes.count indexType:MTLIndexTypeUInt32 indexBuffer:state->fb->cur.verts.idxs->buff indexBufferOffset:c->drawIndexes.iFirst * 4];
                            SCN_PRINTF_VERB("drawIndexedPrimitives(MTLPrimitiveTypeTriangleStrip: %u, +%u).\n", c->drawIndexes.iFirst, c->drawIndexes.count);
                        }
                        break;
                    case ENScnRenderShape_TriangStrip: //triangles-strip, most common shape
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_TriangStrip::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawIndexedPrimitives:MTLPrimitiveTypeTriangleStrip indexCount:c->drawIndexes.count indexType:MTLIndexTypeUInt32 indexBuffer:state->fb->cur.verts.idxs->buff indexBufferOffset:c->drawIndexes.iFirst * 4];
                            SCN_PRINTF_VERB("drawIndexedPrimitives(MTLPrimitiveTypeTriangleStrip: %u, +%u).\n", c->drawIndexes.iFirst, c->drawIndexes.count);
                        }
                        break;
                    //case ENScnRenderShape_TriangFan:   //triangles-fan
                    //    break;
                        //
                    case ENScnRenderShape_LineStrip:   //lines-strip
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_LineStrip::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawIndexedPrimitives:MTLPrimitiveTypeLineStrip indexCount:c->drawIndexes.count indexType:MTLIndexTypeUInt32 indexBuffer:state->fb->cur.verts.idxs->buff indexBufferOffset:c->drawIndexes.iFirst * 4];
                            SCN_PRINTF_VERB("drawIndexedPrimitives(MTLPrimitiveTypeLineStrip: %u, +%u).\n", c->drawIndexes.iFirst, c->drawIndexes.count);
                        }
                        break;
                    //case ENScnRenderShape_LineLoop:    //lines-loop
                    //    break;
                    case ENScnRenderShape_Lines:       //lines
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_Lines::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawIndexedPrimitives:MTLPrimitiveTypeLine indexCount:c->drawIndexes.count indexType:MTLIndexTypeUInt32 indexBuffer:state->fb->cur.verts.idxs->buff indexBufferOffset:c->drawIndexes.iFirst * 4];
                            SCN_PRINTF_VERB("drawIndexedPrimitives(MTLPrimitiveTypeLine: %u, +%u).\n", c->drawIndexes.iFirst, c->drawIndexes.count);
                        }
                        break;
                    case ENScnRenderShape_Points:      //points
                        if(state->rndrEnc == NULL){
                            SCN_PRINTF_ERROR("ENScnRenderShape_Points::rndrEnc == NULL.\n");
                            r = ScnFALSE;
                        } else {
                            [state->rndrEnc drawIndexedPrimitives:MTLPrimitiveTypePoint indexCount:c->drawIndexes.count indexType:MTLIndexTypeUInt32 indexBuffer:state->fb->cur.verts.idxs->buff indexBufferOffset:c->drawIndexes.iFirst * 4];
                            SCN_PRINTF_VERB("drawIndexedPrimitives(MTLPrimitiveTypePoint: %u, +%u).\n", c->drawIndexes.iFirst, c->drawIndexes.count);
                        }
                        break;
                    default:
                        SCN_ASSERT(ScnFALSE); //missing implementation
                        break;
                }
                break;
            default:
                SCN_ASSERT(ScnFALSE) //missing implementation
                break;
        }
        ++c;
    }
    return r;
}

ScnBOOL ScnApiDX12_renderJob_buildEndAndEnqueue(void* data){
    ScnBOOL r = ScnFALSE;
    STScnApiDX12RenderJob* obj = (STScnApiDX12RenderJob*)data;
    STScnApiDX12RenderJobState* state = &obj->state;
    if(state->rndrEnc == nullptr || obj->cmdsBuff == nullptr || state->fb == NULL){
        return ScnFALSE;
    }
    //
    r = ScnTRUE;
    //finalize
    if(state->rndrEnc != nullptr){
        [state->rndrEnc endEncoding];
        SCN_PRINTF_VERB("endEncoding.\n");
    }
    if(obj->cmdsBuff != nullptr){
        if(state->fb != NULL && state->fb->mtkView != NULL){
            [obj->cmdsBuff presentDrawable:state->fb->mtkView.currentDrawable];
            SCN_PRINTF_VERB("presentDrawable.\n");
        }
        [obj->cmdsBuff commit];
        SCN_PRINTF_VERB("commit.\n");
    }
    return r;
}


// STScnApiDX12RenderJobState

void ScnApiDX12RenderJobState_init(STScnApiDX12RenderJobState* obj){
    ScnMemory_setZeroSt(*obj);
}

void ScnApiDX12RenderJobState_destroy(STScnApiDX12RenderJobState* obj){
    if(obj->rndrDesc != nullptr){ [obj->rndrDesc release]; obj->rndrDesc = nullptr; }
    if(obj->rndrEnc != nullptr){ [obj->rndrEnc release]; obj->rndrEnc = nullptr; }
}

void ScnApiDX12RenderJobState_reset(STScnApiDX12RenderJobState* obj){
    if(obj->rndrDesc != nullptr){ [obj->rndrDesc release]; obj->rndrDesc = nullptr; }
    if(obj->rndrEnc != nullptr){ [obj->rndrEnc release]; obj->rndrEnc = nullptr; }
    ScnMemory_setZeroSt(*obj);
}
