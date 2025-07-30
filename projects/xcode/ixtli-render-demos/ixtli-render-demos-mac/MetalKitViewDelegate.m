//
//  MetalKitViewDelegate.m
//  ixtli-render-demos-mac
//
//  Created by Marcos Ortega on 28/7/25.
//

#import "MetalKitViewDelegate.h"

#include "ixrender/ScnRender.h"
#include "ixrender/api/ScnApiMetal.h"
#include "utils/ScnMemMap.h"

// Custom memory allocation for this demos and tests,
// for detecting memory-leaks using ScnMemMap.
void* ScnMemMap_custom_malloc(const ScnUI32 newSz, const char* dbgHintStr);
void* ScnMemMap_custom_realloc(void* ptr, const ScnUI32 newSz, const char* dbgHintStr);
void ScnMemMap_custom_free(void* ptr);
STScnMemMap* gMemmap = NULL;

NS_ASSUME_NONNULL_BEGIN

#define IXTLI_RENDER_DEMO_RENDER_SLOTS_AMMOUNT  3
#define IXTLI_FRAMES_COUNT_AND_RELEASE_RENDER   120

@implementation MetalKitViewDelegate {
    ScnContextRef ctx;
    ScnRenderRef  render;
    ScnModelRef   model;
    //memory leak detection
    STScnMemMap     memmap;
    ScnUI32         framesCount;
@protected
    MTKView         *metalKitView;
}

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view
{
    ctx             = (ScnContextRef)ScnContextRef_Zero;
    render          = (ScnRenderRef)ScnObjRef_Zero;
    model           = (ScnModelRef)ScnObjRef_Zero;
    metalKitView    = nil;

    self = [super init];
    if (nil == self) { return nil; }

    metalKitView = view;

    //Init memory custom manager
    ScnMemMap_init(&memmap);
    gMemmap = &memmap;
    
    STScnContextItf ctxItf;
    memset(&ctxItf, 0, sizeof(ctxItf));
    //define context interface
    {
        //custom memory allocation (for memory leaks dtection)
        {
            //mem
            ctxItf.mem.malloc   = ScnMemMap_custom_malloc;
            ctxItf.mem.realloc  = ScnMemMap_custom_realloc;
            ctxItf.mem.free     = ScnMemMap_custom_free;
        }
        //use default for others
        ScnContextItf_fillMissingMembers(&ctxItf);
    }
    ctx = ScnContext_alloc(&ctxItf);
    if(ScnContext_isNull(ctx)){
        printf("ERROR, ScnContext_alloc failed.\n");
    } else {
        // Do any additional setup after loading the view.
        STScnApiItf itf;
        if(!ScnApiMetal_getApiItf(&itf)){
            printf("ERROR, ScnApiMetal_getApiItf failed.\n");
        } else {
            render = ScnRender_alloc(ctx);
            if(ScnRender_isNull(render)){
                printf("ERROR, ScnRender_alloc failed.\n");
            } else if(!ScnRender_prepare(render, &itf, NULL)){
                printf("ERROR, ScnRender_prepare failed.\n");
            } else if(!ScnRender_openDevice(render, NULL, IXTLI_RENDER_DEMO_RENDER_SLOTS_AMMOUNT)){
                printf("ERROR, ScnRender_openDevice failed.\n");
            } else {
                printf("Render initialized.\n");
                model = ScnRender_allocModel(render);
                if(!ScnModel_isNull(model)){
                    STScnVertexPtr verts = ScnModel_addDraw(model, ENScnRenderShape_TriangStrip, 3);
                    if(verts.ptr != NULL){
                        STScnVertex v;
                        v = verts.ptr[0]; v.x = 10; v.y = 10; v.color.r = 255; v.color.g = 55; v.color.b = 155; v.color.a = 255;
                        v = verts.ptr[1]; v.x = -10; v.y = 10; v.color.r = 255; v.color.g = 55; v.color.b = 155; v.color.a = 255;
                        v = verts.ptr[2]; v.x = 0; v.y = 0; v.color.r = 255; v.color.g = 55; v.color.b = 155; v.color.a = 255;
                    }
                }
            }
        }
    }
    return self;
}

/// Notifies the app when the system adjusts the size of its viewable area.
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
    NSLog(@"MetalKitViewDelegate, mtkView:drawableSizeWillChange(%f, %f).\n", size.width, size.height);
}

/// Notifies the app when the system is ready draw a frame into a view.
- (void)drawInMTKView:(nonnull MTKView *)view
{
    //NSLog(@"MetalKitViewDelegate, drawInMTKView.\n");
    if(!ScnRender_isNull(render) && ScnRender_hasOpenDevice(render)){
        if(!ScnRender_prepareNextRenderSlot(render)){
            printf("ScnRender_prepareNextRenderSlot failed.\n");
        } else if(!ScnRender_jobStart(render)){
            printf("ScnRender_jobStart failed.\n");
        } else {
            if(!ScnRender_jobEnd(render)){
                printf("ScnRender_jobEnd failed.\n");
            }
        }
    }
    //
    if(framesCount < IXTLI_FRAMES_COUNT_AND_RELEASE_RENDER){
        ++framesCount;
        if((framesCount % 120) == 0){
            printf("%d frames.\n", framesCount);
        }
        if(framesCount >= IXTLI_FRAMES_COUNT_AND_RELEASE_RENDER){
            printf("Simulating cleanup after %d frames.\n", framesCount);
            ScnModel_releaseAndNullify(&model);
            ScnRender_releaseAndNullify(&render);
            ScnContext_releaseAndNullify(&ctx);
            //custom memory manager
            {
                ScnMemMap_printFinalReport(&memmap);
                ScnMemMap_destroy(&memmap);
            }
        }
    }
}

@end

NS_ASSUME_NONNULL_END

//custom memory allocation (for memory leaks detection)

void* ScnMemMap_custom_malloc(const ScnUI32 newSz, const char* dbgHintStr){
    void* r = malloc(newSz);
    if(gMemmap == NULL){
        SCN_PRINTF_ERROR("ScnMemMap_custom_malloc::gCurInstance is NULL.\n");
    } else {
        ScnMemMap_ptrAdd(gMemmap, r, newSz, dbgHintStr);
    }
    return r;
}

void* ScnMemMap_custom_realloc(void* ptr, const ScnUI32 newSz, const char* dbgHintStr){
    //"If there is not enough memory, the old memory block is not freed and null pointer is returned."
    void* r = realloc(ptr, newSz);
    if(gMemmap == NULL){
        SCN_PRINTF_ERROR("ScnMemMap_custom_realloc::gCurInstance is NULL.\n");
    } else {
        if(r != NULL){
            if(ptr != NULL){
                ScnMemMap_ptrRemove(gMemmap, ptr);
            }
            ScnMemMap_ptrAdd(gMemmap, r, newSz, dbgHintStr);
        }
    }
    return r;
}

void ScnMemMap_custom_free(void* ptr){
    if(gMemmap == NULL){
        SCN_PRINTF_ERROR("ScnMemMap_custom_free::gCurInstance is NULL.\n");
    } else {
        ScnMemMap_ptrRemove(gMemmap, ptr);
    }
    free(ptr);
}
