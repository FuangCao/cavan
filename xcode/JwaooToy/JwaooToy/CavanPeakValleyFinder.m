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

        mFreqValueList = [NSMutableArray new];
    }

    return self;
}

- (BOOL)isValidPeakValley:(NSTimeInterval)interval {
    double diff = mPeakValue - mValleyValue;

    if (diff > mValueFuzz) {
        return true;
    }

    // NSLog(@"interval = %f, mTimeFuzz = %f", interval, mTimeFuzz);

    if (diff > mValueFuzzMin && interval > mTimeFuzz) {
        return true;
    }

    return false;
}

- (CavanPeakValleyValue *)createPeakValleyValue:(CavanPeakValleyValueType)type {
    if (mType == type) {
        return nil;
    }

    mType = type;

    return [[CavanPeakValleyValue alloc] initWithPeak:mLastPeak withValley:mLastValley withType:type withDate:mLastDate];
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
                result = [self createPeakValleyValue:CavanPeakValleyValueFalling];
                mLastPeak = mPeakValue;
                mLastDate = mPeakDate;
            }
        }
    } else if (mFindValley) {
        if (value > mValue) {
            mFindPeak = true;
            mValleyValue = mValue;
            mValleyDate = [NSDate date];

            if ([self isValidPeakValley:[mValleyDate timeIntervalSinceDate:mPeakDate]]) {
                result = [self createPeakValleyValue:CavanPeakValleyValueRising];
                mLastValley = mValleyValue;
                mLastDate = mValleyDate;
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
        mPeakDate = mValleyDate = mLastDate = [NSDate date];
        mPeakValue = mValleyValue = mLastPeak = mLastValley = value;
    }

    mValue = value;

    return result;
}

- (int)putFreqValue:(double)value {
    NSDate *date;
    CavanPeakValleyValue *result = [self putValue:value];
    if (result == nil) {
        date = [NSDate date];

        while (1) {
            CavanPeakValleyValue *first = [mFreqValueList firstObject];
            if (first == nil) {
                break;
            }

            if ([first timeIntervalEarly:date] < CAVAN_PEAK_VALLEY_FREQ_TIMEOUT) {
                break;
            }

            [mFreqValueList removeObjectAtIndex:0];
        }
    } else {
        date = result.date;
        mAvgDiff = (mAvgDiff + result.getDiff) / 2;

        while (mFreqValueList.count > CAVAN_PEAK_VALLEY_FREQ_COUNT) {
            [mFreqValueList removeObjectAtIndex:0];
        }

        [mFreqValueList addObject:result];
    }

    if (mFreqValueList.count < 2) {
        mFreq = 0;
    } else {
        CavanPeakValleyValue *first = [mFreqValueList firstObject];
        mFreq = 60 * (mFreqValueList.count - 1) / [first timeIntervalEarly:date] / 2;
    }

    return mFreq;
}

@end
