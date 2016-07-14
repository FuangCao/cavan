//
//  JwaooToyParser.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/14.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "JwaooToyParser.h"

@implementation JwaooToyParser

@synthesize depth = mDepth;

- (CavanAccelFreqParser *)initWithValueFuzz:(double)valueFuzz
                                       withTimeFuzz:(NSTimeInterval)timeFuzz
                                       withDelegate:(nullable id<JwaooToyParserDelegate>)delegate {
    if (self = [super initWithValueFuzz:valueFuzz withTimeFuzz:timeFuzz withDelegate:delegate]) {
        mDelegate2 = delegate;
    }

    return self;
}

- (void)onDepthChanged:(int)depth {
    [mDelegate2 didDepthChanged:depth];
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
