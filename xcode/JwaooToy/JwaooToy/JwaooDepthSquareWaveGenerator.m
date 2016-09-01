//
//  JwaooDepthSquareWaveGenerator.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/9/1.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "JwaooDepthSquareWaveGenerator.h"
#import "JwaooToySensor.h"

@implementation JwaooDepthSquareWaveGenerator

- (JwaooDepthSquareWaveGenerator *)initWithFuzz:(double)fuzz
                                   withTimeMin:(NSTimeInterval)timeMin
                                   withTimeMax:(NSTimeInterval)timeMax {
    if (self = [super initWithFuzz:fuzz withTimeMin:timeMin withTimeMax:timeMax]) {
        mPredictedMin = JWAOO_TOY_CAPACITY_MAX;
        mPredictedMax = JWAOO_TOY_CAPACITY_MIN;
        mCapacityMin = JWAOO_TOY_CAPACITY_MIN;
        mCapacityMax = JWAOO_TOY_CAPACITY_MAX;
        mCapacityFuzz = fuzz / 2;

        mCountedArrayMin = [[CavanCountedArray alloc] initByMax:500];
        mCountedArrayMax = [[CavanCountedArray alloc] initByMax:500];
    }

    return self;
}

- (void)setCapacityFuzz:(double)fuzz {
    mCapacityFuzz = fuzz;
}

- (void)savePredictedMin {
    mHasPredictedMin = true;
    mPredictedMin = mCapacity;
}

- (void)savePredictedMax {
    mHasPredictedMax = true;
    mPredictedMax = mCapacity;
}

- (double)getPredictedRange {
    if (mHasPredictedMin && mHasPredictedMax) {
        return mPredictedMax - mPredictedMin;
    }

    return 0;
}

- (void)setPredictedRange:(double)range {
    if (mHasPredictedMax) {
        if (!mHasPredictedMin) {
            mPredictedMin = mPredictedMax - range;
        }
    } else if (mHasPredictedMin) {
        mPredictedMax = mPredictedMin + range;
    }
}

- (double)getPredictedValue {
    if (mCapacity >= mPredictedMax) {
        return 1.0;
    } else if (mCapacity <= mPredictedMin) {
        return 0.0;
    } else {
        return (mCapacity - mCapacityMin) / (mPredictedMax - mPredictedMin);
    }
}

- (bool)isValidMin:(double)min
          andMax:(double)max {
    if (max - min < mValueFuzz) {
        return false;
    }

    return true;
}

- (void)updateThretholdWithMin:(double)min withMax:(double)max {
    if ([self isValidMin:min andMax:max]) {
        mCapacityMin = [mCountedArrayMin putCountedValue:min];
        mCapacityMax = [mCountedArrayMax putCountedValue:max];
    } else if (min < mCapacityMin) {
        mCapacityMin = min;
    } else {
        return;
    }

    mThresholdHigh = (mCapacityMin + mCapacityMax + mCapacityFuzz) / 2;
    mThresholdLow = mThresholdHigh - mCapacityFuzz;

    double low = mCapacityMin + mCapacityFuzz;
    if (mThresholdLow < low) {
        mThresholdLow = low;
    }
}

- (bool)putCapacityValue:(double)value {
    mCapacity = value;

    bool result = [super putSquareValue:value];

    double min = mMinNode.value;
    double max = mMaxNode.value;

    if ([self isValidMin:min andMax:max]) {
        [mCountedArrayMin putCountedValue:min];
        [mCountedArrayMax putCountedValue:max];
    }

    return result;
}

@end
