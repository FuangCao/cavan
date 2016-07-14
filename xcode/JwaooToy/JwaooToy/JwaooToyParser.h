//
//  JwaooToyParser.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/14.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CavanAccelFreqParser.h"
#import "JwaooToySensor.h"

@protocol JwaooToyParserDelegate <CavanAccelFreqParserDelegate>
@required
- (void)didDepthChanged:(int)depth;
@end

@interface JwaooToyParser : CavanAccelFreqParser {
    int mDepth;
    id<JwaooToyParserDelegate> mDelegate2;
}

@property int depth;

- (nonnull CavanAccelFreqParser *)initWithValueFuzz:(double)valueFuzz
                                       withTimeFuzz:(NSTimeInterval)timeFuzz
                                       withDelegate:(nullable id<JwaooToyParserDelegate>)delegate;
- (void)putSensorData:(nonnull JwaooToySensor *)sensor;

@end
