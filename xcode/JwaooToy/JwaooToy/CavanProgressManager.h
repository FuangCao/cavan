//
//  CavanProgressMamager.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/9.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface CavanProgressManager : NSObject {
    double mValue;
    double mValueMin;
    double mValueMax;
    double mValueRange;

    int mProgress;
    int mProgressMin;
    int mProgressMax;
    int mProgressRange;

    SEL mProgressSelector;
    NSObject *mProgressTarget;
}

- (nonnull CavanProgressManager *)initWithSelector:(nonnull SEL)selector
                                withTarget:(nullable NSObject *)target;
- (nonnull CavanProgressManager *)initWithProgressMin:(int)min
                                      withMax:(int)max;
- (nonnull CavanProgressManager *)initWithProgressRange:(int)range;
- (void)setProgressSelector:(nonnull SEL)selector
                 withTarget:(nullable NSObject *)target;
- (void)onProgressUpdated:(int)progress;
- (void)setProgressMax:(int)max;
- (void)setProgressMin:(int)min
               withMax:(int)max;
- (void)setProgressRange:(int)range;
- (void)setProgress:(int)progress;
- (void)setProgressPersent:(int)persent;
- (void)setDouble:(double)persent;
- (void)addProgress;
- (void)addProgressWithValue:(int)value;
- (void)finish;

- (void)setValueMin:(double)min
            withMax:(double)max;
- (void)setValueRange:(double)range;
- (void)setValue:(double)value;
- (void)addValue:(double)value;

@end
