//
//  Mpu6050Sensor.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/8.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CavanAccelSensor.h"

@interface JwaooToySensorDefault : CavanAccelSensor

+ (double)buildAxisWithValue8:(int8_t)value;
+ (double)buildAxisWithValue16:(int16_t)value;

@end
