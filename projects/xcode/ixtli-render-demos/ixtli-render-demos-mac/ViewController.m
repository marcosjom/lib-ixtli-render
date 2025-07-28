//
//  ViewController.m
//  ixtli-render-demos-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#import "ViewController.h"
#import "MetalKitViewDelegate.h"

@implementation ViewController {
    STScnContextRef         ctx;
    STScnRenderRef          render;
    //
    MTKView                 *view;
    MetalKitViewDelegate    *delegate;
}


- (void)viewDidLoad {
    [super viewDidLoad];
    //
    ctx             = (STScnContextRef)STScnContextRef_Zero;
    render          = (STScnRenderRef)STScnObjRef_Zero;
    //
    view            = (MTKView *)self.view;
    delegate        = [[MetalKitViewDelegate alloc] initWithMetalKitView:view];
    view.delegate   = delegate;
    //
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
            } else if(!ScnRender_openDevice(render, NULL)){
                printf("ERROR, ScnRender_createDevice failed.\n");
            } else {
                printf("Render initialized.\n");
            }
        }
    }
}


- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}


@end
