//
//  ViewController.m
//  hellotexture-mac
//
//  Created by Marcos Ortega on 23/8/25.
//

#import "ViewController.h"
#import "MTKViewDelegateHelloTexture.h"

@implementation ViewController {
    MTKView                     *view;
    MTKViewDelegateHelloTexture *delegate;
}

- (void)viewDidLoad {
    [super viewDidLoad];

    // Do any additional setup after loading the view.
    view            = (MTKView *)self.view;
    delegate        = [[MTKViewDelegateHelloTexture alloc] initWithMetalKitView:view];
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
