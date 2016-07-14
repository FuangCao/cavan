//
//  CavanAccelSensor.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/13.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface CavanAccelSensor : NSObject {
    double mAxisX;
    double mAxisY;
    double mAxisZ;
}

@property double x;
@property double y;
@property double z;

- (nonnull CavanAccelSensor *)initWithBytes:(nonnull const void *)bytes;
- (void)putBytes:(nonnull const void *)bytes;

@end
