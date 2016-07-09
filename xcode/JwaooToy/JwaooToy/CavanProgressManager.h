//
//  CavanProgressMamager.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/9.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol CavanProgressDelegate <NSObject>
@required
- (void)didProgressUpdated:(nonnull NSNumber *)progress;
@end

@interface CavanProgressManager : NSObject {
    double mValue;
    double mValueMin;
    double mValueMax;
    double mValueRange;

    int mProgress;
    int mProgressMin;
    int mProgressMax;
    int mProgressRange;

    id<CavanProgressDelegate> mDelegate;
}

@property (nonnull) id<CavanProgressDelegate> delegate;

- (nonnull CavanProgressManager *)initWithDelegate:(nonnull id<CavanProgressDelegate>)delegate;
- (nonnull CavanProgressManager *)initWithProgressMin:(int)min
                                      withMax:(int)max;
- (nonnull CavanProgressManager *)initWithProgressRange:(int)range;

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
