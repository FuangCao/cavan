//
//  JwaooToySensor.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/14.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "JwaooToySensor.h"

@implementation JwaooToySensor

- (int)depth {
    return mDepth;
}

- (void)setCapacity:(int)index
              value:(int)value {
    mCapacitys[index] = value;
}

- (void)setCapacityWithBytes:(const int8_t *)bytes {
    for (int i = 0; i < 4; i++) {
        mCapacitys[i] = bytes[i];
    }

    [self updateDepth];
}

- (void)updateDepth {
    double depth = 0;

    for (int i = 0; i < 4; i++) {
        if (mCapacitys[i] > 0) {
            depth += mCapacitys[i];
        }
    }

    mDepth = (mDepth + (depth / 4)) / 2;
}

@end
