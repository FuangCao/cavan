//
//  CavanSquareWaveGenerator.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/9/1.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "CavanSquareWaveGenerator.h"

@implementation CavanSquareWaveGenerator

@synthesize square = mSquareValue;
@synthesize fuzz = mValueFuzz;
@synthesize timeMin = mTimeMin;

- (CavanSquareWaveGenerator *)initWithFuzz:(double)fuzz
                              withTimeMin:(NSTimeInterval)timeMin
                              withTimeMax:(NSTimeInterval)timeMax {
    if (self = [super initByOvertime:timeMax]) {
        mValueFuzz = fuzz;
        mTimeMin = timeMin;
        [self setThrethold:CAVAN_SQUARE_THRETHOLD_DEFAULT];
    }

    return self;
}

- (void)setThrethold:(double)threthold {
    if (threthold > 1) {
        mThreshold = 1;
    } else {
        mThreshold = threthold;
    }

    if (mMinNode != nil && mMaxNode != nil) {
        [self onExtremeUpdated];
    }
}

- (void)onExtremeUpdated {
    mValueRange = mMaxNode.value - mMinNode.value;

    if (self.getTimeInterval < mTimeMin) {
        mThresholdLow = mMaxNode.value - mValueFuzz;
        mThresholdHigh = mMinNode.value + mValueFuzz;
    } else {
        [self updateThretholdWithMin:mMinNode.value withMax:mMaxNode.value];
    }
}

- (void)updateThretholdWithMin:(double)min
                      withMax:(double)max {
    double fuzz = mValueRange * mThreshold;
    if (fuzz < mValueFuzz) {
        fuzz = mValueFuzz;
    }

    mThresholdHigh = (min + max + fuzz) / 2;
    mThresholdLow = mThresholdHigh - fuzz;

    min += mValueFuzz / 2;
    if (mThresholdLow < min) {
        mThresholdLow = min;
    }
}

- (bool)putSquareValue:(double)value {
    [self putWaveValue:value];

    if (value > mThresholdHigh) {
        mSquareValue = true;
    } else if (value < mThresholdLow) {
        mSquareValue = false;
    }

    return mSquareValue;
}

@end
