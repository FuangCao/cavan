//
//  CavanTimedList.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/8/31.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "CavanTimedArray.h"

@implementation CavanTimedNode

@synthesize time = mTime;

- (CavanTimedNode *)initByTime:(NSTimeInterval)time {
    if (self = [super init]) {
        mTime = time;
    }

    return self;
}

- (CavanTimedNode *)init {
    return [self initByTime:[NSDate timeIntervalSinceReferenceDate]];
}

@end

@implementation CavanTimedArray

@synthesize overtime = mOvertime;
@synthesize freq = mFreq;
@synthesize cycle = mCycle;

- (CavanTimedArray *)initByOvertime:(NSTimeInterval)overtime {
    if (self = [super init]) {
        mOvertime = overtime;
        mNodes = [NSMutableArray new];
    }

    return self;
}

- (void)removeTimedNode:(NSInteger)index {
    [mNodes removeObjectAtIndex:index];
}

- (CavanTimedNode *)getFirstNode {
    return mNodes.firstObject;
}

- (CavanTimedNode *)getLastNode {
    return mNodes.lastObject;
}

- (NSTimeInterval)getFirstTime {
    CavanTimedNode *node = self.getFirstNode;
    if (node != nil) {
        return node.time;
    }

    return 0;
}

- (NSTimeInterval)getLastTime {
    CavanTimedNode *node = self.getLastNode;
    if (node != nil) {
        return node.time;
    }

    return 0;
}

- (NSTimeInterval)getTimeInterval {
    return self.getLastTime - self.getFirstTime;
}

- (NSUInteger)removeOvertimeNodes:(NSTimeInterval)timeLast {
    NSUInteger count = 0;

    while (1) {
        CavanTimedNode *firstNode = mNodes.firstObject;
        if (firstNode == nil || firstNode.time > timeLast) {
            break;
        }

        [self removeTimedNode:0];
        count++;
    }

    return count;
}

- (NSUInteger)removeOvertimeNodes {
    NSTimeInterval timeLast = [NSDate timeIntervalSinceReferenceDate] - mOvertime;
    return [self removeOvertimeNodes:timeLast];
}

- (void)addTimedNode:(CavanTimedNode *)node {
    NSTimeInterval timeLast = node.time - mOvertime;
    [self removeOvertimeNodes:timeLast];
    [mNodes addObject:node];
}

- (double)updateFreq {
    NSUInteger count = mNodes.count;
    if (count > 1) {
        CavanTimedNode *firstNode = mNodes.firstObject;
        CavanTimedNode *lastNode = mNodes.lastObject;

        mCycle = (lastNode.time - firstNode.time) / (count - 1);

        NSTimeInterval timeNow = [NSDate timeIntervalSinceReferenceDate];
        NSTimeInterval interval = timeNow - lastNode.time;
        if (interval < mCycle) {
            mFreq = 1.0 / mCycle;
        } else {
            mFreq = count / (timeNow - firstNode.time);
        }
    } else if (count < 1) {
        mFreq = 0;
    }

    return mFreq;
}

@end
