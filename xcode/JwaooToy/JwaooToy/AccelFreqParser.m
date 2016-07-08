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
        mFinderBetter = mFinderX;
    }

    return self;
}

- (void)setDepth:(int)depth {
    depth = (mDepth + depth) / 2;
    if (mDepth != depth) {
        mDepth = depth;
        [mDelegate didDepthChanged:depth];
    }
}

- (void)setFreq:(int)freq {
    if (freq > 0) {
        freq = (mFreq + freq) / 2;
    }

    if (mFreq != freq) {
        mFreq = freq;
        [mDelegate didFreqChanged:freq];
    }
}

- (CavanPeakValleyFinder *)putBytes:(const int8_t *)bytes {
    [mSensor setValueWithBytes8:bytes];
    [mFinderX putFreqValue:mSensor.x];
    [mFinderY putFreqValue:mSensor.y];
    [mFinderZ putFreqValue:mSensor.z];

    if (mFinderX.diff > mFinderY.diff) {
        if (mFinderX.diff > mFinderZ.diff) {
            mFinderBetter = mFinderX;
        } else {
            mFinderBetter = mFinderZ;
        }
    } else if (mFinderY.diff > mFinderZ.diff) {
        mFinderBetter = mFinderY;
    } else {
        mFinderBetter = mFinderZ;
    }

    [self setFreq:mFinderBetter.freq];
    [self setDepth:bytes[3]];

    return mFinderBetter;
}

@end
