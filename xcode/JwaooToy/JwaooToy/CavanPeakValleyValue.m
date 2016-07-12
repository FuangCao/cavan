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
@synthesize date = mDate;
@synthesize peak = mPeakValue;
@synthesize valley = mValleyValue;
@synthesize interval = mInterval;

- (CavanPeakValleyValue *)initWithPeak:(double)peak
                            withValley:(double)valley
                              withType:(CavanPeakValleyValueType)type
                              withDate:(NSDate *)date {
    if (self = [super init]) {
        mType = type;
        mPeakValue = peak;
        mValleyValue = valley;
        mDate = date;
    }

    return self;
}

- (CavanPeakValleyValue *)initWithPeak:(double)peak
                            withValley:(double)valley
                              withType:(CavanPeakValleyValueType)type {
    return [self initWithPeak:peak withValley:valley withType:type withDate:[NSDate date]];
}

- (CavanPeakValleyValue *)initWithValue:(double)value {
    return [self initWithPeak:value withValley:value withType:CavanPeakValleyValueFalling];
}

- (CavanPeakValleyValue *)copy {
    return [[CavanPeakValleyValue alloc] initWithPeak:mPeakValue withValley:mValleyValue withType:mType];
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

- (NSTimeInterval)timeIntervalLate:(NSDate *)date {
    return [mDate timeIntervalSinceDate:date];
}

- (NSTimeInterval)timeIntervalEarly:(NSDate *)date {
    return [date timeIntervalSinceDate:mDate];
}

- (NSTimeInterval)timeIntervalLateWithValue:(CavanPeakValleyValue *)value {
    return [self timeIntervalLate:value.date];
}

- (NSTimeInterval)timeIntervalEarlyWithValue:(CavanPeakValleyValue *)value {
    return [self timeIntervalEarly:value.date];
}
@end
