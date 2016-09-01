//
//  CavanSquareWaveCounter.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/9/1.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "CavanSquareWaveCounter.h"

@implementation CavanSquareWaveCounter

- (double)freq {
    return mFreqArray.freq;
}

- (double)cycle {
    return mFreqArray.cycle;
}

- (void)setFreqOvertime:(NSTimeInterval)overtime {
    mFreqArray.overtime = overtime;
}

- (CavanSquareWaveCounter *)initWithFuzz:(double)fuzz
                     withMinTime:(NSTimeInterval)minTime
                    withValueOvertime:(NSTimeInterval)valueOvertime
                     withFreqOvertime:(NSTimeInterval)freqOvertime {
    if (self = [super initWithFuzz:fuzz withTimeMin:minTime withTimeMax:valueOvertime]) {
        mFreqArray = [[CavanTimedArray alloc] initByOvertime:freqOvertime];
    }

    return self;
}

- (double)updateFreq {
    mFreq = mFreqArray.updateFreq;
    return mFreq;
}

- (double)putFreqValue:(double)value {
    bool result = [self putSquareValue:value];

    if (result && mLastSquareValue == false) {
        [mFreqArray addTimedNode:self.getLastNode];
        [self updateFreq];
    } else if ([mFreqArray removeOvertimeNodes] > 0) {
        [self updateFreq];
    }

    mLastSquareValue = result;

    return mFreq;
}

@end
