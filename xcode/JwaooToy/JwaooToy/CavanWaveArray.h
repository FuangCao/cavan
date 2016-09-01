//
//  CavanWaveArray.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/8/31.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CavanTimedArray.h"

@interface CavanWaveNode : CavanTimedNode {
    double mValue;
}

@property double value;

@end

@interface CavanWaveArray : CavanTimedArray {
    bool mNeedFind;
    CavanWaveNode *mMaxNode;
    CavanWaveNode *mMinNode;
}

- (CavanWaveArray *)initByOvertime:(NSTimeInterval)overtime;
- (void)removeTimedNode:(NSInteger)index;
- (void)updateExtremeNode;
- (void)putWaveValue:(double)value;
- (void)onExtremeUpdated;

@end
