//
//  CavanPeakValleyValue.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/8.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef NS_OPTIONS(uint8_t, CavanPeakValleyValueType) {
    CavanPeakValleyValueFalling     = 0,
    CavanPeakValleyValueRising      = 1,
};

@interface CavanPeakValleyValue : NSObject {
    double mPeakValue;
    double mValleyValue;
    NSTimeInterval mTime;
    CavanPeakValleyValueType mType;
}

@property double peak;
@property double valley;
@property NSTimeInterval time;
@property CavanPeakValleyValueType type;

- (CavanPeakValleyValue *)initWithPeak:(double)peak
                            withValley:(double)valley
                              withType:(CavanPeakValleyValueType)type
                              withTime:(NSTimeInterval)time;
- (CavanPeakValleyValue *)initWithPeak:(double)peak
                            withValley:(double)valley
                              withType:(CavanPeakValleyValueType)type;
- (CavanPeakValleyValue *)initWithValue:(double)value;


- (CavanPeakValleyValue *)copy;
- (double)getDiff;
- (BOOL)isFalling;
- (BOOL)isRising;
- (NSTimeInterval)timeIntervalSinceTime:(NSTimeInterval)time;
- (NSTimeInterval)timeIntervalSince:(CavanPeakValleyValue *)value;
- (NSTimeInterval)timeIntervalSinceDate:(NSDate *)date;
@end
