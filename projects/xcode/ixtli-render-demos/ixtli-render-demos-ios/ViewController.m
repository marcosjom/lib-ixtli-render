//
//  ViewController.m
//  ixtli-render-demos-ios
//
//  Created by Marcos Ortega on 1/8/25.
//

#import "ViewController.h"
#import "MetalKitViewDelegate.h"

@interface ViewController () {
    MTKView                 *view;
    MetalKitViewDelegate    *delegate;
}
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    view            = (MTKView *)self.view;
    delegate        = [[MetalKitViewDelegate alloc] initWithMetalKitView:view];
    view.delegate   = delegate;
}

- (void) touchesEnded:(NSSet<UITouch *> *) touches withEvent:(UIEvent *) event {
    [delegate startOrStop];
}

@end
