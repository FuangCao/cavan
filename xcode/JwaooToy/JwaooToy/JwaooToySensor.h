//
//  JwaooToySensor.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/14.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CavanAccelSensor.h"

@interface JwaooToySensor : CavanAccelSensor {
    double mDepth;
    int mCapacitys[4];
}

@property (readonly) int depth;

- (void)setCapacity:(int)index
              value:(int)value;
- (void)setCapacityWithBytes:(const int8_t *)bytes;
- (void)updateDepth;

@end
