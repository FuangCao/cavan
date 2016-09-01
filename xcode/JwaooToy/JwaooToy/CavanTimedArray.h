//
//  CavanTimedList.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/8/31.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface CavanTimedNode : NSObject {
    NSTimeInterval mTime;
}

@property NSTimeInterval time;

- (CavanTimedNode *)initByTime:(NSTimeInterval)time;
- (CavanTimedNode *)init;

@end

@interface CavanTimedArray : NSObject {
    double mFreq;
    double mCycle;
    NSTimeInterval mOvertime;
    NSMutableArray *mNodes;
}

@property NSTimeInterval overtime;
@property double freq;
@property double cycle;

- (CavanTimedArray *)initByOvertime:(NSTimeInterval)overtime;
- (void)removeTimedNode:(NSInteger)index;
- (NSUInteger)removeOvertimeNodes:(NSTimeInterval)timeLast;
- (NSUInteger)removeOvertimeNodes;
- (void)addTimedNode:(CavanTimedNode *)node;
- (double)updateFreq;
- (CavanTimedNode *)getFirstNode;
- (CavanTimedNode *)getLastNode;
- (NSTimeInterval)getFirstTime;
- (NSTimeInterval)getLastTime;
- (NSTimeInterval)getTimeInterval;

@end
