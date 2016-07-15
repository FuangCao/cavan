//
//  CavanPeakValleyValue.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/8.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "CavanPeakValleyValue.h"

@implementation CavanPeakValleyValue

@synthesize type = mType;
@synthesize time = mTime;
@synthesize peak = mPeakValue;
@synthesize valley = mValleyValue;

- (CavanPeakValleyValue *)initWithPeak:(double)peak
                            withValley:(double)valley
                              withType:(CavanPeakValleyValueType)type
                              withTime:(NSTimeInterval)time {
    if (self = [super init]) {
        mType = type;
        mTime = time;
        mPeakValue = peak;
        mValleyValue = valley;
    }

    return self;
}

- (CavanPeakValleyValue *)initWithPeak:(double)peak
                            withValley:(double)valley
                              withType:(CavanPeakValleyValueType)type {
    return [self initWithPeak:peak withValley:valley withType:type withTime:[NSDate timeIntervalSinceReferenceDate]];
}

- (CavanPeakValleyValue *)initWithValue:(double)value {
    return [self initWithPeak:value withValley:value withType:CavanPeakValleyValueFalling];
}

- (CavanPeakValleyValue *)copy {
    return [[self.class alloc] initWithPeak:mPeakValue withValley:mValleyValue withType:mType];
}

- (NSString *)description {
    if ([self isRising]) {
        return [NSString stringWithFormat:@"Rising: [%f, %f]", mValleyValue, mPeakValue];
    } else {
        return [NSString stringWithFormat:@"Falling: [%f, %f]", mPeakValue, mValleyValue];
    }
}

- (double)getDiff {
    return mPeakValue - mPeakValue;
}

- (BOOL)isFalling {
    return mType == CavanPeakValleyValueFalling;
}

- (BOOL)isRising {
    return mType == CavanPeakValleyValueRising;
}

- (NSTimeInterval)timeIntervalSinceTime:(NSTimeInterval)time {
    return mTime - time;
}

- (NSTimeInterval)timeIntervalSince:(CavanPeakValleyValue *)value {
    return [self timeIntervalSinceTime:value.time];
}

- (NSTimeInterval)timeIntervalSinceDate:(NSDate *)date {
    return [self timeIntervalSinceTime:[date timeIntervalSinceReferenceDate]];
}

@end
