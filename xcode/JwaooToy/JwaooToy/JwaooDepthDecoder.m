//
//  JwaooDepthDecoder.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/9/1.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "JwaooDepthDecoder.h"

@implementation JwaooDepthDecoder

@synthesize depth = mDepth;

- (JwaooDepthDecoder *)initWithFuzz:(double)fuzz {
    if (self = [super initWithFuzz:fuzz withMinTime:JWAOO_DEPTH_DECODER_TIME_MIN withValueOvertime:JWAOO_DEPTH_DECODER_TIME_MAX_VALUE withFreqOvertime:JWAOO_DEPTH_DECODER_TIME_MAX_FREQ]) {
        for (int i = 0; i < JWAOO_TOY_CAPACITY_SENSOR_COUNT; i++) {
            mGenerators[i] = [[JwaooDepthSquareWaveGenerator alloc] initWithFuzz:fuzz withTimeMin:JWAOO_DEPTH_DECODER_TIME_MIN withTimeMax:JWAOO_DEPTH_DECODER_TIME_MAX_VALUE];
        }
    }

    return self;
}

- (void)setFuzz:(double)fuzz {
    [super setFuzz:fuzz];

    for (int i = 0; i < JWAOO_TOY_CAPACITY_SENSOR_COUNT; i++) {
        mGenerators[i].fuzz = fuzz;
    }
}

- (void)setTimeMin:(NSTimeInterval)timeMin {
    [super setTimeMin:timeMin];

    for (int i = 0; i < JWAOO_TOY_CAPACITY_SENSOR_COUNT; i++) {
        mGenerators[i].timeMin = timeMin;
    }
}

- (void)setOvertime:(NSTimeInterval)overtime {
    [super setOvertime:overtime];

    for (int i = 0; i < JWAOO_TOY_CAPACITY_SENSOR_COUNT; i++) {
        mGenerators[i].overtime = overtime;
    }
}

- (void)updateThretholdWithMin:(double)min withMax:(double)max {
    if (mValueRange > mValueFuzz) {
        mThresholdHigh = (min + max + mValueFuzz) / 2;
        mThresholdLow = mThresholdHigh - mValueFuzz;
    }
}

- (JwaooDepthSquareWaveGenerator *)getGeneratorAtIndex:(int)index {
    return mGenerators[index];
}

- (double)updateDepthWithCount:(int)count {
    if (count < JWAOO_TOY_CAPACITY_SENSOR_COUNT) {
        if (count > 0) {
            return 0.5 + count - 1;
        } else {
            return 0;
        }
    } else {
        return JWAOO_TOY_CAPACITY_SENSOR_COUNT;
    }
}

- (double)getPredictedDepthWithCount:(int)count {
    double depth = mGenerators[count - 1].getPredictedValue + mGenerators[count].getPredictedValue - 0.5;

    if (depth > 1) {
        return 1;
    } else if (depth < 0) {
        return 0;
    } else {
        return depth;
    }
}

- (double)putCapacityValue:(double *)capacitys {
    double sum = 0;

    for (int i = 0; i < JWAOO_TOY_CAPACITY_SENSOR_COUNT; i++) {
        sum += capacitys[i];
    }

    if ([self putFreqValue:sum] > 0) {
        [self setOvertime:(self.cycle * 2)];
    } else {
        [self setOvertime:JWAOO_DEPTH_DECODER_TIME_MAX_VALUE];
    }

    int count = JWAOO_TOY_CAPACITY_SENSOR_COUNT;

    for (int i = JWAOO_TOY_CAPACITY_SENSOR_COUNT - 1; i >= 0; i--) {
        if (![mGenerators[i] putCapacityValue:capacitys[i]]) {
            count = i;
        }
    }

    double depth;

    if (count > mCount || count <= 0) {
        mPlugIn = true;
        depth = mDepthAlign = [self updateDepthWithCount:count];
    } else if (count < mCount) {
        mPlugIn = false;
        depth = mDepthAlign = [self updateDepthWithCount:count];
    } else {
        if (mPlugIn) {
            for (int i = count + 1; i < JWAOO_TOY_CAPACITY_SENSOR_COUNT; i++) {
                [mGenerators[i] savePredictedMin];
            }

            for (int i = count - 2; i >= 0; i--) {
                [mGenerators[i] savePredictedMax];
            }
        }

        if (count < JWAOO_TOY_CAPACITY_SENSOR_COUNT) {
            if (count > 0) {
                depth = [self getPredictedDepthWithCount:count] + mDepthAlign;
            } else {
                depth = 0;
            }
        } else {
            depth = JWAOO_TOY_CAPACITY_SENSOR_COUNT;
        }
    }

    mCount = count;
    mDepth = depth / JWAOO_TOY_CAPACITY_SENSOR_COUNT;

    return mDepth;
}

@end
