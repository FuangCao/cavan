//
//  JwaooDepthSquareWaveGenerator.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/9/1.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CavanCountedArray.h"
#import "CavanSquareWaveGenerator.h"

@interface JwaooDepthSquareWaveGenerator : CavanSquareWaveGenerator {
    bool mHasPredictedMin;
    bool mHasPredictedMax;
    double mPredictedMin;
    double mPredictedMax;

    double mCapacity;
    double mCapacityFuzz;
    double mCapacityMin;
    double mCapacityMax;

    CavanCountedArray *mCountedArrayMin;
    CavanCountedArray *mCountedArrayMax;
}

- (JwaooDepthSquareWaveGenerator *)initWithFuzz:(double)fuzz
                                   withTimeMin:(NSTimeInterval)timeMin
                                   withTimeMax:(NSTimeInterval)timeMax;

- (void)setCapacityFuzz:(double)fuzz;
- (void)savePredictedMin;
- (void)savePredictedMax;
- (double)getPredictedRange;
- (void)setPredictedRange:(double)range;
- (double)getPredictedValue;
- (bool)isValidMin:(double)min
           andMax:(double)max;
- (void)updateThretholdWithMin:(double)min withMax:(double)max;
- (bool)putCapacityValue:(double)value;

@end
