//
//  AccelFreqParser.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/8.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CavanPeakValleyFinder.h"
#import "CavanAccelSensor.h"

@protocol AccelFreqParserDelegate <NSObject>
@required
- (void)didFreqChanged:(int)freq;
- (void)didDepthChanged:(int)depth;
@end

@interface AccelFreqParser : NSObject {
    int mFreq;
    int mDepth;
    id<AccelFreqParserDelegate> mDelegate;

    CavanPeakValleyFinder *mFinderX;
    CavanPeakValleyFinder *mFinderY;
    CavanPeakValleyFinder *mFinderZ;
    CavanPeakValleyFinder *mFinderBest;
}

@property (readonly) int freq;
@property (readonly) int depth;

- (nonnull AccelFreqParser *)initWithValueFuzz:(double)valueFuzz
                          withTimeFuzz:(NSTimeInterval)timeFuzz
                          withDelegate:(nullable id<AccelFreqParserDelegate>)delegate;
- (void)putSensorData:(nonnull CavanAccelSensor *)sensor;

@end
