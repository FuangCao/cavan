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

@protocol CavanAccelFreqParserDelegate <NSObject>
@required
- (void)didFreqChanged:(int)freq;
@end

@interface CavanAccelFreqParser : NSObject {
    int mFreq;
    id<CavanAccelFreqParserDelegate> mDelegate;

    CavanPeakValleyFinder *mFinderX;
    CavanPeakValleyFinder *mFinderY;
    CavanPeakValleyFinder *mFinderZ;
    CavanPeakValleyFinder *mFinderBest;
}

@property (readonly) int freq;

- (nonnull CavanAccelFreqParser *)initWithValueFuzz:(double)valueFuzz
                          withTimeFuzz:(NSTimeInterval)timeFuzz
                          withDelegate:(nullable id<CavanAccelFreqParserDelegate>)delegate;
- (void)onFreqChanged:(int)freq;
- (void)putSensorData:(nonnull CavanAccelSensor *)sensor;

@end
