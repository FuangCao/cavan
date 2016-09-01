//
//  JwaooToyParser.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/14.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "JwaooToySensor.h"
#import "JwaooDepthDecoder.h"

@interface JwaooToyParser : JwaooDepthDecoder

-(void)putSensorData:(JwaooToySensor *)sensor;

@end
