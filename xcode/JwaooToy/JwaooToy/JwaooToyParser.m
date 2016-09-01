//
//  JwaooToyParser.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/14.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "JwaooToyParser.h"

@implementation JwaooToyParser

-(void)putSensorData:(JwaooToySensor *)sensor {
    [super putCapacityValue:sensor.getCapacitys];
}

@end
