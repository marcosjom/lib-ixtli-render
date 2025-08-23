//
//  ViewController.m
//  hellotriang-ios
//
//  Created by Marcos Ortega on 23/8/25.
//

#import "ViewController.h"
#import "MTKViewDelegateHelloTriang.h"

@interface ViewController () {
    MTKView                     *view;
    MTKViewDelegateHelloTriang  *delegate;
}

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    view            = (MTKView *)self.view;
    delegate        = [[MTKViewDelegateHelloTriang alloc] initWithMetalKitView:view];
    view.delegate   = delegate;
}

- (void) touchesEnded:(NSSet<UITouch *> *) touches withEvent:(UIEvent *) event {
    [delegate startOrStop];
}

@end
