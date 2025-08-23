//
//  ViewController.m
//  hellotriang-mac
//
//  Created by Marcos Ortega on 23/8/25.
//

#import "ViewController.h"
#import "MTKViewDelegateHelloTriang.h"

@implementation ViewController {
    MTKView                     *view;
    MTKViewDelegateHelloTriang   *delegate;
}

- (void)viewDidLoad {
    [super viewDidLoad];

    // Do any additional setup after loading the view.
    view            = (MTKView *)self.view;
    delegate        = [[MTKViewDelegateHelloTriang alloc] initWithMetalKitView:view];
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
