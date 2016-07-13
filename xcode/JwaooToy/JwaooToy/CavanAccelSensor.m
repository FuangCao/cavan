//
//  CavanAccelSensor.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/13.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "CavanAccelSensor.h"

@implementation CavanAccelSensor

@synthesize x;
@synthesize y;
@synthesize z;

- (CavanAccelSensor *)initWithBytes:(const void *)bytes {
    if (self = [super init]) {
        [self parseBytes:bytes];
    }

    return self;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"[%f, %f, %f]", x, y, z];
}

- (void)parseBytes:(const void *)bytes {
    x = y = z = 0;
}

@end
