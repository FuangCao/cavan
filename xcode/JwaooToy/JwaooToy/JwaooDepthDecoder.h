//
//  JwaooDepthDecoder.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/9/1.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "JwaooToySensor.h"
#import "CavanSquareWaveCounter.h"
#import "JwaooDepthSquareWaveGenerator.h"

#define JWAOO_DEPTH_DECODER_TIME_MIN            0.8
#define JWAOO_DEPTH_DECODER_TIME_MAX_VALUE      2
#define JWAOO_DEPTH_DECODER_TIME_MAX_FREQ       5

@interface JwaooDepthDecoder : CavanSquareWaveCounter {
    bool mPlugIn;
    NSUInteger mCount;

    double mDepth;
    double mDepthAlign;

    JwaooDepthSquareWaveGenerator *mGenerators[JWAOO_TOY_CAPACITY_SENSOR_COUNT];
}

@property double depth;

- (JwaooDepthDecoder *)initWithFuzz:(double)fuzz;
- (void)updateThretholdWithMin:(double)min withMax:(double)max;
- (JwaooDepthSquareWaveGenerator *)getGeneratorAtIndex:(int)index;
- (double)updateDepthWithCount:(int)count;
- (double)getPredictedDepthWithCount:(int)count;
- (double)putCapacityValue:(double *)capacitys;

@end
