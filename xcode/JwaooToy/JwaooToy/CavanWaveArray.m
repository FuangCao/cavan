//
//  CavanWaveArray.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/8/31.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "CavanWaveArray.h"

@implementation CavanWaveNode

@synthesize value = mValue;

- (CavanWaveNode *)initByValue:(double)value {
    if (self = [super init]) {
        mValue = value;
    }

    return self;
}

@end

@implementation CavanWaveArray

- (CavanWaveArray *)initByOvertime:(NSTimeInterval)overtime {
    if (self = [super initByOvertime:overtime]) {
        mNeedFind = true;
    }

    return self;
}

- (void)removeTimedNode:(NSInteger)index {
    CavanTimedNode *node = [mNodes objectAtIndex:index];
    if (node == mMaxNode || node == mMinNode) {
        mNeedFind = true;
    }

    [super removeTimedNode:index];
}

- (void)onExtremeUpdated {
}

- (void)updateExtremeNode {
    mMinNode = mMaxNode = mNodes.firstObject;

    for (NSInteger index = mNodes.count - 1; index > 0; index--) {
        CavanWaveNode *node = [mNodes objectAtIndex:index];
        if (node.value > mMaxNode.value) {
            mMaxNode = node;
        } else if (node.value < mMinNode.value) {
            mMinNode = node;
        }
    }
}

- (void)putWaveValue:(double)value {
    CavanWaveNode *node = [[CavanWaveNode alloc] initByValue:value];

    [self addTimedNode:node];

    if (mNeedFind) {
        mNeedFind = false;
        [self updateExtremeNode];
    } else if (value > mMaxNode.value) {
        mMinNode = node;
    } else if (value < mMinNode.value) {
        mMaxNode = node;
    } else {
        return;
    }

    [self onExtremeUpdated];
}

@end
