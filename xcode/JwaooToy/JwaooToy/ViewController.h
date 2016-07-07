//
//  ViewController.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/7.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "JwaooBleToy.h"

@interface ViewController : NSViewController {
    BOOL mSensorEnable;
    JwaooBleToy *mBleToy;
}


@end

