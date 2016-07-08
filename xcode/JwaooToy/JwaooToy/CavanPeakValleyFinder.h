//
//  CavanPeakValleyFinder.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/8.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CavanPeakValleyValue.h"

#define CAVAN_PEAK_VALLEY_FREQ_COUNT    10
#define CAVAN_PEAK_VALLEY_FREQ_TIMEOUT  2

@interface CavanPeakValleyFinder : CavanPeakValleyValue {
    BOOL mInitialized;
    BOOL mFindPeak;
    BOOL mFindValley;

    double mAvgDiff;

    double mValue;
    double mAvgValue;

    double mValueFuzz;
    double mValueFuzzMin;
    NSTimeInterval mTimeFuzz;

    double mLastPeak;
    NSDate *mPeakDate;

    double mLastValley;
    NSDate *mValleyDate;

    int mFreq;
    NSDate *mLastDate;
    NSMutableArray *mValueList;
}

@property (readonly) double diff;
@property (readonly) int freq;

- (nullable CavanPeakValleyFinder *)initWithValueFuzz:(double)valueFuzz
                                withTimeFuzz:(NSTimeInterval)timeFuzz;
- (BOOL)isValidPeakValley:(NSTimeInterval)interval;
- (nullable CavanPeakValleyValue *)createValue:(CavanPeakValleyValueType)type;
- (nullable CavanPeakValleyValue *)putValue:(double)value;
- (int)putFreqValue:(double)value;
@end
