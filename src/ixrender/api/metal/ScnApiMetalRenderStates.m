//
//  ScnApiMetalRenderStates.m
//  ixtli-render
//
//  Created by Marcos Ortega on 18/8/25.
//

#include "ScnApiMetalRenderStates.h"

// STScnApiMetalRenderStates

void STScnApiMetalRenderStates_init(STScnApiMetalRenderStates* obj){
    ScnMemory_setZeroSt(*obj);
}

void STScnApiMetalRenderStates_destroy(STScnApiMetalRenderStates* obj){
    //states
    {
        id<MTLRenderPipelineState>* s = &obj->states[0];
        id<MTLRenderPipelineState>* sAfterEnd = s + (sizeof(obj->states) / sizeof(obj->states[0]));
        while(s < sAfterEnd){
            if(*s != nil){
                [*s release];
                *s  = nil;
            }
            ++s;
        }
    }
}

//

ScnBOOL STScnApiMetalRenderStates_load(STScnApiMetalRenderStates* obj, STScnApiMetalDevice* dev, MTLPixelFormat color){
    ScnBOOL r = ScnTRUE;
    obj->color = color;
    {
        id<MTLFunction> vertexFunc = nil;
        id<MTLFunction> fragmtFunc = nil;
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
                SCN_PRINTF_ERROR("STScnApiMetalRenderStates_load unimplemented function name.\n");
                r = ScnFALSE;
                break;
            }
            if(fragmtFuncName == NULL){
                SCN_PRINTF_ERROR("STScnApiMetalRenderStates_load unimplemented function name.\n");
                r = ScnFALSE;
                break;
            }
            //
            if(vertexFunc != nil){ [vertexFunc release]; vertexFunc = nil; }
            if(fragmtFunc != nil){ [fragmtFunc release]; fragmtFunc = nil; }
            //
            vertexFunc = [dev->lib newFunctionWithName:[NSString stringWithUTF8String:vertexFuncName]];
            if(vertexFunc == nil){
                SCN_PRINTF_ERROR("STScnApiMetalRenderStates_load newFunctionWithName('%s') failed.\n", vertexFuncName);
                r = ScnFALSE;
                break;
            }
            fragmtFunc = [dev->lib newFunctionWithName:[NSString stringWithUTF8String:fragmtFuncName]];
            if(fragmtFunc == nil){
                SCN_PRINTF_ERROR("STScnApiMetalRenderStates_load newFunctionWithName('%s') failed.\n", fragmtFuncName);
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
                if(nil == (obj->states[i] = [dev->dev newRenderPipelineStateWithDescriptor:rndrPipeDesc error:&error])){
                    SCN_PRINTF_ERROR("STScnApiMetalRenderStates_load::newRenderPipelineStateWithDescriptor failed: '%s'.\n", (error == nil ? "nil" : [[error description] UTF8String]));
                    r = ScnFALSE;
                    break;
                }
            }
        }
        if(vertexFunc != nil){ [vertexFunc release]; vertexFunc = nil; }
        if(fragmtFunc != nil){ [fragmtFunc release]; fragmtFunc = nil; }
    }
    return r;
}
