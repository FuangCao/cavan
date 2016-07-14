//
//  CavanAccelFreqParser.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/8.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CavanPeakValleyFinder.h"
#import "CavanAccelSensor.h"

@interface CavanAccelFreqParser : NSObject {
    int mFreq;
    SEL mFreqSelector;
    NSObject *mFreqTarget;

    CavanPeakValleyFinder *mFinderX;
    CavanPeakValleyFinder *mFinderY;
    CavanPeakValleyFinder *mFinderZ;
    CavanPeakValleyFinder *mFinderBest;
}

@property (readonly) int freq;

- (nonnull CavanAccelFreqParser *)initWithValueFuzz:(double)valueFuzz
                          withTimeFuzz:(NSTimeInterval)timeFuzz;
- (void)setFreqSelector:(nonnull SEL)selector
             withTarget:(nullable NSObject *)target;
- (void)onFreqChanged:(int)freq;
- (void)putSensorData:(nonnull CavanAccelSensor *)sensor;

@end
