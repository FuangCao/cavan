//
//  AccelFreqParser.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/8.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "AccelFreqParser.h"

@implementation AccelFreqParser

@synthesize freq = mFreq;
@synthesize depth = mDepth;

- (AccelFreqParser *)initWithValueFuzz:(double)valueFuzz
                          withTimeFuzz:(NSTimeInterval)timeFuzz
                          withDelegate:(id)delegate
{
    if (self = [super init]) {
        mDelegate = delegate;
        mSensor = [Mpu6050Sensor new];
        mFinderX = [[CavanPeakValleyFinder alloc] initWithValueFuzz:valueFuzz withTimeFuzz:timeFuzz];
        mFinderY = [[CavanPeakValleyFinder alloc] initWithValueFuzz:valueFuzz withTimeFuzz:timeFuzz];
        mFinderZ = [[CavanPeakValleyFinder alloc] initWithValueFuzz:valueFuzz withTimeFuzz:timeFuzz];
        mFinderBest = mFinderX;
    }

    return self;
}

- (void)updateDepth:(int)depth {
    depth = (mDepth + depth) / 2;
    if (mDepth != depth) {
        mDepth = depth;
        [mDelegate didDepthChanged:depth];
    }
}

- (void)updateFreq:(int)freq {
    if (freq > 0 && mFreq > 0) {
        freq = (mFreq + freq) / 2;
    }

    if (mFreq != freq) {
        mFreq = freq;
        [mDelegate didFreqChanged:freq];
    }
}

- (void)putBytes:(const int8_t *)bytes {
    [mSensor setValueWithBytes8:bytes];

    [mFinderX putFreqValue:mSensor.x];
    [mFinderY putFreqValue:mSensor.y];
    [mFinderZ putFreqValue:mSensor.z];

    if (mFinderX.diff > mFinderY.diff) {
        if (mFinderX.diff > mFinderZ.diff) {
            mFinderBest = mFinderX;
        } else {
            mFinderBest = mFinderZ;
        }
    } else if (mFinderY.diff > mFinderZ.diff) {
        mFinderBest = mFinderY;
    } else {
        mFinderBest = mFinderZ;
    }

    [self updateFreq:mFinderBest.freq];
    [self updateDepth:bytes[3]];
}

@end
