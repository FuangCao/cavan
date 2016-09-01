//
//  CavanSquareWaveCounter.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/9/1.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CavanSquareWaveGenerator.h"

@interface CavanSquareWaveCounter : CavanSquareWaveGenerator {
    bool mLastSquareValue;
    CavanTimedArray *mFreqArray;
}

- (CavanSquareWaveCounter *)initWithFuzz:(double)fuzz
                            withMinTime:(NSTimeInterval)minTime
                      withValueOvertime:(NSTimeInterval)valueOvertime
                       withFreqOvertime:(NSTimeInterval)freqOvertime;

- (void)setFreqOvertime:(NSTimeInterval)overtime;
- (double)putFreqValue:(double)value;

@end
