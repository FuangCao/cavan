//
//  CavanCountedList.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/8/31.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "CavanCountedArray.h"

@implementation CavanCountedNode

@synthesize count = mCount;
@synthesize value = mValue;

- (CavanCountedNode *)initByValue:(NSInteger)value {
    if (self = [super init]) {
        mCount = 1;
        mValue = value;
    }

    return self;
}

- (NSUInteger)decrement {
    if (mCount > 0) {
        mCount--;
    }

    return mCount;
}

- (NSUInteger)increment:(NSUInteger)max {
    if (mCount < max) {
        mCount++;
    }

    return mCount;
}

@end

@implementation CavanCountedArray

@synthesize max = mMaxCount;

- (CavanCountedArray *)initByMax:(NSUInteger)max {
    if (self = [super init]) {
        mMaxCount = max;
        mNodes = [NSMutableArray new];
    }

    return self;
}

- (CavanCountedNode *)addCountedValue:(NSInteger)value {
    CavanCountedNode *freeNode = nil;

    for (NSInteger index = mNodes.count - 1; index >= 0; index--) {
        CavanCountedNode *node = [mNodes objectAtIndex:index];

        if (node.value == value) {
            CavanCountedNode *foundNode = node;

            [foundNode increment:mMaxCount];

            while (--index >= 0) {
                node = [mNodes objectAtIndex:index];
                if (node.decrement == 0) {
                    [mNodes removeObjectAtIndex:index];
                }
            }

            return foundNode;
        } else if (node.decrement == 0) {
            if (freeNode == nil) {
                freeNode = node;
            } else {
                [mNodes removeObjectAtIndex:index];
            }
        }
    }

    if (freeNode == nil) {
        freeNode = [[CavanCountedNode alloc] initByValue:value];
        [mNodes addObject:freeNode];
    } else {
        freeNode.count = 1;
        freeNode.value = value;
    }

    return freeNode;
}

- (CavanCountedNode *)getBestNode {
    NSInteger count = 0;
    CavanCountedNode *bestNode = nil;

    for (CavanCountedNode *node in mNodes) {
        if (node.count > count) {
            count = node.count;
            bestNode = node;
        }
    }

    return bestNode;
}

- (NSInteger)getBestValue {
    CavanCountedNode *node = [self getBestNode];
    if (node == nil) {
        return 0;
    }

    return node.value;
}

- (NSInteger)putCountedValue:(NSInteger)value {
    [self addCountedValue:value];

    CavanCountedNode *node = self.getBestNode;

    return node.value;
}

@end
