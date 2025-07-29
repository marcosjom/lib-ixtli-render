//
//  MetalKitViewDelegate.m
//  ixtli-render-demos-mac
//
//  Created by Marcos Ortega on 28/7/25.
//

#import "MetalKitViewDelegate.h"

#include "ixrender/ScnRender.h"
#include "ixrender/api/ScnApiMetal.h"

NS_ASSUME_NONNULL_BEGIN

#define IXTLI_RENDER_DEMO_RENDER_SLOTS_AMMOUNT  3

@implementation MetalKitViewDelegate {
    STScnContextRef ctx;
    STScnRenderRef  render;
    STScnModelRef   model;
@protected
    MTKView         *metalKitView;
}

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view
{
    ctx             = (STScnContextRef)STScnContextRef_Zero;
    render          = (STScnRenderRef)STScnObjRef_Zero;
    model           = (STScnModelRef)STScnObjRef_Zero;
    metalKitView    = nil;

    self = [super init];
    if (nil == self) { return nil; }

    metalKitView = view;

    STScnContextItf ctxItf = ScnContextItf_getDefault();
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
    if(!ScnRender_isNull(render)){
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
}

@end

NS_ASSUME_NONNULL_END
