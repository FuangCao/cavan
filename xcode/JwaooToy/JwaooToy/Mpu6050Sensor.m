//
//  Mpu6050Sensor.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/8.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "Mpu6050Sensor.h"

@implementation Mpu6050Sensor

@synthesize x;
@synthesize y;
@synthesize z;

+ (double)buildAxisWithValue8:(int8_t)value {
    return value * 9.8 / 64;
}

+ (double)buildAxisWithValue16:(int16_t)value {
    return value * 9.8 / 16384;
}

- (Mpu6050Sensor *)initWithBytes8:(const int8_t *)bytes {
    if (self = [super init]) {
        [self setValueWithBytes8:bytes];
    }

    return self;
}

- (Mpu6050Sensor *)initWithBytes16:(const int16_t *)bytes {
    if (self = [super init]) {
        [self setValueWithBytes16:bytes];
    }

    return self;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"[%f, %f, %f]", x, y, z];
}

- (void)setValueWithBytes8:(const int8_t *)bytes {
    x = [self.class buildAxisWithValue8:bytes[0]];
    y = [self.class buildAxisWithValue8:bytes[1]];
    z = [self.class buildAxisWithValue8:bytes[2]];
}

- (void)setValueWithBytes16:(const int16_t *)bytes {
    x = [self.class buildAxisWithValue16:bytes[0]];
    y = [self.class buildAxisWithValue16:bytes[1]];
    z = [self.class buildAxisWithValue16:bytes[2]];
}

@end
