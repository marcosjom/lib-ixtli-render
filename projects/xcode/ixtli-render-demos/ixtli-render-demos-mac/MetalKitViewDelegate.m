//
//  MetalKitViewDelegate.m
//  ixtli-render-demos-mac
//
//  Created by Marcos Ortega on 28/7/25.
//

#import "MetalKitViewDelegate.h"

NS_ASSUME_NONNULL_BEGIN

@implementation MetalKitViewDelegate {
    
@protected
    MTKView *metalKitView;
}

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view
{
    metalKitView = nil;

    self = [super init];
    if (nil == self) { return nil; }

    metalKitView = view;

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
}

@end

NS_ASSUME_NONNULL_END
