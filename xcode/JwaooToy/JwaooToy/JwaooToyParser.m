//
//  JwaooToyParser.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/14.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "JwaooToyParser.h"

#pragma clang diagnostic ignored "-Warc-performSelector-leaks"

@implementation JwaooToyParser

@synthesize depth = mDepth;

- (void)setDepthSelector:(SEL)selector
              withTarget:(NSObject *)target {
    mDepthSelector = selector;
    mDepthTarget = target;
}

- (void)onDepthChanged:(int)depth {
    [mDepthTarget performSelector:mDepthSelector withObject:[NSNumber numberWithInt:depth]];
}

- (void)updateDepth:(int)depth {
    if (depth != mDepth) {
        mDepth = depth;
        [self onDepthChanged:depth];
    }
}

- (void)putSensorData:(nonnull JwaooToySensor *)sensor {
    [super putSensorData:sensor];
    [self updateDepth:sensor.depth];
}

@end
