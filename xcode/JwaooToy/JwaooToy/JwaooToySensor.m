//
//  JwaooToySensor.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/14.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "JwaooToySensor.h"

@implementation JwaooToySensor

- (void)setCapacity:(int)index
              value:(double)value {
    mCapacitys[index] = value;
}

-(double *)getCapacitys {
    return mCapacitys;
}

-(double)getCapacityAtIndex:(int)index {
    return mCapacitys[index];
}

-(void)setCapacitysWithBytes8:(const int8_t *)values {
    for (int i = 0; i < JWAOO_TOY_CAPACITY_SENSOR_COUNT; i++) {
        mCapacitys[i] = values[i];
    }
}

-(void)setCapacitysWithBytes16:(const uint8_t *)values {
    for (int i = 0, j = 0; i < JWAOO_TOY_CAPACITY_SENSOR_COUNT; i++, j += 2) {
        int16_t value = (((int16_t) values[j] << 8) | values[j + 1]);
        mCapacitys[i] = ((double) value) / 256;
    }
}

@end
