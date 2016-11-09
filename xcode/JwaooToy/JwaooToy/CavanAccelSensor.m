//
//  CavanAccelSensor.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/13.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "CavanAccelSensor.h"

@implementation CavanAccelSensor

@synthesize x = mAxisX;
@synthesize y = mAxisY;
@synthesize z = mAxisZ;

- (CavanAccelSensor *)initWithBytes:(const void *)bytes {
    if (self = [super init]) {
        [self putBytes:bytes];
    }

    return self;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"[%7.2f, %7.2f, %7.2f]", mAxisX, mAxisY, mAxisZ];
}

- (void)putBytes:(const void *)bytes {
    mAxisX = mAxisY = mAxisZ = 0;
}

@end
