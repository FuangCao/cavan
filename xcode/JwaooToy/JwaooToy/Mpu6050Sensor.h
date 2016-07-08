//
//  Mpu6050Sensor.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/8.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Mpu6050Sensor : NSObject {
    double x;
    double y;
    double z;
}

@property double x;
@property double y;
@property double z;

+ (double)buildAxisWithValue8:(int8_t)value;
+ (double)buildAxisWithValue16:(int16_t)value;

- (Mpu6050Sensor *)initWithBytes8:(const int8_t *)bytes;
- (Mpu6050Sensor *)initWithBytes16:(const int16_t *)bytes;
- (void)setValueWithBytes8:(const int8_t *)bytes;
- (void)setValueWithBytes16:(const int16_t *)bytes;

@end
