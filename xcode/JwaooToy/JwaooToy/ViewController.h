//
//  ViewController.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/7.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "JwaooBleToy.h"

@interface ViewController : NSViewController <CavanProgressDelegate, JwaooBleToyDelegate> {
    BOOL mSensorEnable;
    JwaooBleToy *mBleToy;
    int mFreq;
    int mDepth;
    int mCount;
}

@property (weak) IBOutlet NSProgressIndicator *mProgressBar;
@property (weak) IBOutlet NSButton *mButtonSensCommand;
@property (weak) IBOutlet NSButton *mButtonSensor;
@property (weak) IBOutlet NSButton *mButtonDisconnect;
@property (weak) IBOutlet NSButton *mButtonReboot;
@property (weak) IBOutlet NSButton *mButtonUpgrade;
@property (weak) IBOutlet NSTextField *mLabelDataCount;
@property (weak) IBOutlet NSTextField *mLabelDataSpeed;
@property (weak) IBOutlet NSTextField *mLabelFreq;
@property (weak) IBOutlet NSTextField *mLabelDepth;

@end

