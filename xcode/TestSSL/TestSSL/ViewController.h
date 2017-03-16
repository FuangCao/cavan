//
//  ViewController.h
//  TestSSL
//
//  Created by 曹福昂 on 17/3/16.
//  Copyright © 2017年 曹福昂. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "GCDAsyncSocket.h"

@interface ViewController : NSViewController <GCDAsyncSocketDelegate> {
    GCDAsyncSocket *mSocket;
}


@end

