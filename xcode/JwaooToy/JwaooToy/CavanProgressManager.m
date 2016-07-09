//
//  CavanProgressMamager.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/9.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "CavanProgressManager.h"

@implementation CavanProgressManager

@synthesize delegate = mDelegate;

- (CavanProgressManager *)initWithDelegate:(id<CavanProgressDelegate>)delegate {
    if (self = [super init]) {
        mDelegate = delegate;
    }

    return self;
}

- (CavanProgressManager *)initWithProgressMin:(int)min
                    withMax:(int)max {
    if (self = [super init]) {
        [self setProgressMin:min withMax:max];
        [self setValueRange:mProgressRange];
    }

    return self;
}

- (CavanProgressManager *)initWithProgressRange:(int)range {
    if (self = [super init]) {
        [self setProgressRange:range];
        [self setValueRange:range];
    }

    return self;
}


- (void)setProgressMax:(int)max {
    mProgressMax = max;
    mProgressRange = max - mProgressMin;
}

- (void)setProgressMin:(int)min
               withMax:(int)max {
    mProgressMin = min;
    [self setProgressMax:max];

    mProgress = min;
    [mDelegate didProgressUpdated:[NSNumber numberWithInt:mProgress]];
}

- (void)setProgressRange:(int)range {
    [self setProgressMin:0 withMax:range];
}

- (void)setProgress:(int)progress {
    if (progress > mProgressMax) {
        progress = mProgressMax;
    } else if (progress < mProgressMin) {
        progress = mProgressMin;
    }

    if (mProgress != progress) {
        mProgress = progress;
        [mDelegate didProgressUpdated:[NSNumber numberWithInt:progress]];
    }
}

- (void)setProgressPersent:(int)persent {
    int progress = mProgressMin + persent * mProgressRange / 100;
    [self setProgress:progress];
}

- (void)setDouble:(double)persent {
    int progress = mProgressMin + mProgressRange * persent;
    [self setProgress:progress];
}

- (void)addProgress {
    [self setProgress:mProgress + 1];
}

- (void)addProgressWithValue:(int)value {
    [self setProgress:mProgress + value];
}

- (void)finish {
    [self setProgress:mProgressMax];
}

- (void)setValueMin:(double)min
            withMax:(double)max {
    mValue = min;
    mValueMin = min;
    mValueMax = max;
    mValueRange = max - min;
    mProgressMin = mProgress;
    mProgressRange = mProgressMax - mProgressMin;
}

- (void)setValueRange:(double)range {
    [self setValueMin:0 withMax:range];
}

- (void)setValue:(double)value {
    if (mValue != value) {
        mValue = value;
        [self setProgress:mProgressMin + (value - mValueMin) * mProgressRange / mValueRange];
    }
}

- (void)addValue:(double)value {
    [self setValue:mValue + value];
}

@end
