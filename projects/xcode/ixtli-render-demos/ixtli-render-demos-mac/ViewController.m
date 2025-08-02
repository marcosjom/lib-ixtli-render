//
//  ViewController.m
//  ixtli-render-demos-mac
//
//  Created by Marcos Ortega on 27/7/25.
//

#import "ViewController.h"
#import "MetalKitViewDelegate.h"

@implementation ViewController {
    MTKView                 *view;
    MetalKitViewDelegate    *delegate;
}


- (void)viewDidLoad {
    [super viewDidLoad];
    //
    view            = (MTKView *)self.view;
    delegate        = [[MetalKitViewDelegate alloc] initWithMetalKitView:view];
    view.delegate   = delegate;
}


- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

- (void)mouseUp:(NSEvent *)theEvent {
    [delegate startOrStop];
}

@end
