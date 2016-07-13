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

        mFreqList = [NSMutableArray new];
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

    return [[CavanPeakValleyValue alloc] initWithPeak:mLastPeak withValley:mLastValley withType:type withTime:mLastTime];
}

- (CavanPeakValleyValue *)putValue:(double)value {
    CavanPeakValleyValue *result = nil;

    if (mFindPeak) {
        if (value < mValue) {
            mFindPeak = false;
            mFindValley = true;
            mPeakValue = mValue;
            mPeakTime = [NSDate timeIntervalSinceReferenceDate];

            if ([self isValidPeakValley:(mPeakTime - mValleyTime)]) {
                result = [self createPeakValleyValue:CavanPeakValleyValueFalling];
                mLastPeak = mPeakValue;
                mLastTime = mPeakTime;
            }
        }
    } else if (mFindValley) {
        if (value > mValue) {
            mFindPeak = true;
            mValleyValue = mValue;
            mValleyTime = [NSDate timeIntervalSinceReferenceDate];

            if ([self isValidPeakValley:(mValleyTime - mPeakTime)]) {
                result = [self createPeakValleyValue:CavanPeakValleyValueRising];
                mLastValley = mValleyValue;
                mLastTime = mValleyTime;
            }
        }
    } else if (mInitialized) {
        if (value > mValue) {
            mFindPeak = true;
        } else if (value < mValue) {
            mFindValley = true;
        }

        mPeakTime = mValleyTime = mLastTime = [NSDate timeIntervalSinceReferenceDate];
    } else {
        mInitialized = true;
        mPeakValue = mValleyValue = mLastPeak = mLastValley = value;
    }

    mValue = value;

    return result;
}

- (CavanPeakValleyValue *)putFreqValue:(double)value {
    BOOL needUpdate;
    CavanPeakValleyValue *result = [self putValue:value];
    if (result != nil) {
        [mFreqList addObject:result];
        needUpdate = true;
    } else {
        needUpdate = false;
    }

    NSTimeInterval lastTime = [NSDate timeIntervalSinceReferenceDate] - CAVAN_PEAK_VALLEY_FREQ_TIMEOUT;

    while (1) {
        CavanPeakValleyValue *first = mFreqList.firstObject;
        if (first == nil || first.time > lastTime) {
            break;
        }

        [mFreqList removeObjectAtIndex:0];
        needUpdate = true;
    }

    if (needUpdate) {
        NSUInteger count = mFreqList.count;
        if (count > 1) {
            NSTimeInterval time;

            if (count < CAVAN_PEAK_VALLEY_FREQ_COUNT) {
                time = [NSDate timeIntervalSinceReferenceDate];
            } else {
                time = ((CavanPeakValleyValue *)mFreqList.lastObject).time;
            }

            mFreq = (count - 1) * 30 / (time - [mFreqList.firstObject time]);
        } else {
            mFreq = 0;
        }
    }

    return result;
}

@end
