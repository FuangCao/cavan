//
//  CavanSquareWaveGenerator.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/9/1.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CavanWaveArray.h"

#define CAVAN_SQUARE_THRETHOLD_DEFAULT  0.25

@interface CavanSquareWaveGenerator : CavanWaveArray {
    double mValueFuzz;
    double mValueRange;
    NSTimeInterval mTimeMin;

    double mThreshold;
    double mThresholdLow;
    double mThresholdHigh;

    bool mSquareValue;
}

@property bool square;
@property double fuzz;
@property NSTimeInterval timeMin;

- (CavanSquareWaveGenerator *)initWithFuzz:(double)fuzz
                              withTimeMin:(NSTimeInterval)timeMin
                              withTimeMax:(NSTimeInterval)timeMax;
- (void)setThrethold:(double)threthold;
- (void)updateThretholdWithMin:(double)min
                      withMax:(double)max;
- (bool)putSquareValue:(double)value;

@end
