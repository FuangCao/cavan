//
//  JwaooToySensorModel10.m
//  JwaooToy
//
//  Created by 曹福昂 on 17/1/2.
//  Copyright © 2017年 曹福昂. All rights reserved.
//

#import "JwaooToySensorModel10.h"

@implementation JwaooToySensorModel10

+ (double)buildAxisWithValue8:(int8_t)value {
    return value * 9.8 / 64;
}

+ (double)buildAxisWithValue16:(int16_t)value {
    return value * 9.8 / 16384;
}

- (void)putBytes:(const void *)bytes {
    const uint16_t *values = bytes;

    mAxisX = [self.class buildAxisWithValue16:values[1]];
    mAxisY = [self.class buildAxisWithValue16:values[0]];
    mAxisZ = [self.class buildAxisWithValue16:values[2]];
}

@end
