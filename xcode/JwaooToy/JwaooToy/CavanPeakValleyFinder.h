//
//  CavanPeakValleyFinder.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/8.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CavanPeakValleyValue.h"

#define CAVAN_PEAK_VALLEY_FREQ_COUNT    5
#define CAVAN_PEAK_VALLEY_FREQ_TIMEOUT  3.0

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
    double mLastValley;

    NSTimeInterval mLastTime;
    NSTimeInterval mPeakTime;
    NSTimeInterval mValleyTime;

    int mFreq;
    NSMutableArray *mFreqList;
}

@property (readonly) double diff;
@property (readonly) int freq;

- (nullable CavanPeakValleyFinder *)initWithValueFuzz:(double)valueFuzz
                                withTimeFuzz:(NSTimeInterval)timeFuzz;
- (BOOL)isValidPeakValley:(NSTimeInterval)interval;
- (nullable CavanPeakValleyValue *)createPeakValleyValue:(CavanPeakValleyValueType)type;
- (nullable CavanPeakValleyValue *)putValue:(double)value;
- (nullable CavanPeakValleyValue *)putFreqValue:(double)value;
@end
