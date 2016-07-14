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

@interface JwaooToyParser : CavanAccelFreqParser {
    int mDepth;
    SEL mDepthSelector;
    NSObject *mDepthTarget;
}

@property int depth;

- (void)setDepthSelector:(nonnull SEL)selector
              withTarget:(nullable NSObject *)target;
- (void)putSensorData:(nonnull JwaooToySensor *)sensor;

@end
