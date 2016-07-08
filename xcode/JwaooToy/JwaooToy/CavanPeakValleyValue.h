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
    NSDate *mDate;
    double mPeakValue;
    double mValleyValue;
    CavanPeakValleyValueType mType;
}

@property CavanPeakValleyValueType type;
@property NSTimeInterval interval;
@property double peak;
@property double valley;
@property NSDate *date;

- (CavanPeakValleyValue *)initWithPeak:(double)peak
                            withValley:(double)valley
                              withType:(CavanPeakValleyValueType)type;
- (CavanPeakValleyValue *)initWithValue:(double)value;


- (CavanPeakValleyValue *)copy;
- (double)getDiff;
- (BOOL)isFalling;
- (BOOL)isRising;
- (NSTimeInterval)timeIntervalLate:(NSDate *)date;
- (NSTimeInterval)timeIntervalEarly:(NSDate *)date;
- (NSTimeInterval)timeIntervalLateWithValue:(CavanPeakValleyValue *)value;
- (NSTimeInterval)timeIntervalEarlyWithValue:(CavanPeakValleyValue *)value;
@end
