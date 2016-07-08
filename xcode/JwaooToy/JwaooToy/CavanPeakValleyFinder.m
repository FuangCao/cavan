//
//  CavanPeakValleyFinder.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/8.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "CavanPeakValleyFinder.h"

@implementation CavanPeakValleyFinder

@synthesize diff = mAvgDiff;
@synthesize freq = mFreq;

- (CavanPeakValleyFinder *)initWithValueFuzz:(double)valueFuzz
                                withTimeFuzz:(NSTimeInterval)timeFuzz {
    if (self = [super initWithValue:0]) {
        mTimeFuzz = timeFuzz;
        mValueFuzz = valueFuzz;
        mValueFuzzMin = valueFuzz / 2;
        mValueList = [NSMutableArray new];
        mLastDate = [NSDate date];
    }

    return self;
}

- (BOOL)isValidPeakValley:(NSTimeInterval)interval {
    double diff = mPeakValue - mValleyValue;

    if (diff < mValueFuzzMin) {
        return false;
    }

    // NSLog(@"interval = %f, mTimeFuzz = %f", interval, mTimeFuzz);

    if (diff < mValueFuzz && interval < mTimeFuzz) {
        return false;
    }

    return true;
}

- (CavanPeakValleyValue *)createValue:(CavanPeakValleyValueType)type {
    if (mType == type) {
        return nil;
    }

    mType = type;

    return [[CavanPeakValleyValue alloc] initWithPeak:mLastPeak withValley:mLastValley withType:type];
}

- (CavanPeakValleyValue *)putValue:(double)value {
    CavanPeakValleyValue *result = nil;

    if (mFindPeak) {
        if (value < mValue) {
            mFindPeak = false;
            mFindValley = true;
            mPeakValue = mValue;
            mPeakDate = [NSDate date];

            if ([self isValidPeakValley:[mPeakDate timeIntervalSinceDate:mValleyDate]]) {
                result = [self createValue:CavanPeakValleyValueFalling];
                mLastPeak = mPeakValue;
            }
        }
    } else if (mFindValley) {
        if (value > mValue) {
            mFindPeak = true;
            mValleyValue = mValue;
            mValleyDate = [NSDate date];

            if ([self isValidPeakValley:[mValleyDate timeIntervalSinceDate:mPeakDate]]) {
                result = [self createValue:CavanPeakValleyValueRising];
                mLastValley = mValleyValue;
            }
        }
    } else if (mInitialized) {
        if (value > mValue) {
            mFindPeak = true;
        } else if (value < mValue) {
            mFindValley = true;
        }
    } else {
        mInitialized = true;
        mPeakDate = mValleyDate = [NSDate date];
        mPeakValue = mValleyValue = mLastPeak = mLastValley = value;
    }

    mValue = value;

    return result;
}

- (int)putFreqValue:(double)value {
    NSDate *date = [NSDate date];
    CavanPeakValleyValue *pvValue = [self putValue:value];
    if (pvValue == nil) {
        if ([date timeIntervalSinceDate:mLastDate] > CAVAN_PEAK_VALLEY_FREQ_TIMEOUT) {
            [mValueList removeAllObjects];
            mAvgDiff = 0;
        }
    } else {
        mLastDate = date;
        [mValueList addObject:pvValue];
        mAvgDiff = (mAvgDiff + pvValue.getDiff) / 2;
    }

    while (mValueList.count > CAVAN_PEAK_VALLEY_FREQ_COUNT) {
        [mValueList removeObjectAtIndex:0];
    }

    if (mValueList.count < 2) {
        mFreq = 0;
    } else {
        pvValue = [mValueList firstObject];
        mFreq = 60 * (mValueList.count - 1) / [pvValue timeIntervalEarly:date] / 2;
    }

    return mFreq;
}

@end
