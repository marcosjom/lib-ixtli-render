//
//  MetalKitViewDelegate.h
//  ixtli-render-demos-mac
//
//  Created by Marcos Ortega on 28/7/25.
//

#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface MetalKitViewDelegate : NSObject<MTKViewDelegate>

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view;

@end

NS_ASSUME_NONNULL_END
