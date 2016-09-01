//
//  JwaooToySensor.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/14.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CavanAccelSensor.h"

#define JWAOO_TOY_CAPACITY_MIN              (-128)
#define JWAOO_TOY_CAPACITY_MAX              (128)
#define JWAOO_TOY_CAPACITY_SENSOR_COUNT     4

@interface JwaooToySensor : CavanAccelSensor {
    double mCapacitys[JWAOO_TOY_CAPACITY_SENSOR_COUNT];
}

- (void)setCapacity:(int)index
              value:(double)value;
-(double *)getCapacitys;
-(double)getCapacityAtIndex:(int)index;
-(void)setCapacitysWithBytes8:(const int8_t *)values;
-(void)setCapacitysWithBytes16:(const uint8_t *)values;

@end
