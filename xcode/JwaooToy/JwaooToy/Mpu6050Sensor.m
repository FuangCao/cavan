//
//  Mpu6050Sensor.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/8.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "Mpu6050Sensor.h"

@implementation Mpu6050Sensor

+ (double)buildAxisWithValue8:(int8_t)value {
    return value * 9.8 / 64;
}

+ (double)buildAxisWithValue16:(int16_t)value {
    return value * 9.8 / 16384;
}

- (void)putBytes:(const void *)bytes {
#if 0
    const uint16_t *values = bytes;

    mAxisX = [self.class buildAxisWithValue16:values[0]];
    mAxisY = [self.class buildAxisWithValue16:values[1]];
    mAxisZ = [self.class buildAxisWithValue16:values[2]];
#else
    const uint8_t *values = bytes;

    mAxisX = [self.class buildAxisWithValue8:values[0]];
    mAxisY = [self.class buildAxisWithValue8:values[1]];
    mAxisZ = [self.class buildAxisWithValue8:values[2]];
    [self setCapacityWithBytes:(bytes + 3)];
#endif
}

@end
