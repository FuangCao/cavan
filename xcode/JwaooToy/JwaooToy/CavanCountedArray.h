//
//  CavanCountedList.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/8/31.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface CavanCountedNode : NSObject {
    NSUInteger mCount;
    NSInteger mValue;
}

@property NSUInteger count;
@property NSInteger value;

- (CavanCountedNode *)initByValue:(NSInteger)value;
- (NSUInteger)decrement;
- (NSUInteger)increment:(NSUInteger)max;

@end

@interface CavanCountedArray : NSObject {
    NSUInteger mMaxCount;
    NSMutableArray *mNodes;
}

@property NSUInteger max;

- (CavanCountedArray *)initByMax:(NSUInteger)max;
- (CavanCountedNode *)addCountedValue:(NSInteger)value;
- (CavanCountedNode *)getBestNode;
- (NSInteger)getBestValue;
- (NSInteger)putCountedValue:(NSInteger)value;

@end
