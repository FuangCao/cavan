//
//  ViewController.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/7.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "JwaooBleToy.h"

@interface ViewController : NSViewController <JwaooBleToyDelegate> {
    BOOL mSensorEnable;
    JwaooBleToy *mBleToy;
    int mCount;
}

@property (weak) IBOutlet NSProgressIndicator *mProgressBar;
@property (weak) IBOutlet NSButton *mButtonSensCommand;
@property (weak) IBOutlet NSButton *mButtonDisconnect;
@property (weak) IBOutlet NSButton *mButtonReboot;
@property (weak) IBOutlet NSButton *mButtonUpgrade;
@property (weak) IBOutlet NSTextField *mLabelDataCount;
@property (weak) IBOutlet NSTextField *mLabelDataSpeed;
@property (weak) IBOutlet NSTextField *mTextFieldBdAddr;
@property (weak) IBOutlet NSButton *mButtonSensor;
@property (weak) IBOutlet NSButton *mButtonLongClick;
@property (weak) IBOutlet NSButton *mButtonMultiClick;
@property (weak) IBOutlet NSButton *mButtonClick;
@property (weak) IBOutlet NSButton *mButtonBattInfo;
@property (weak) IBOutlet NSButton *mButtonReadBdAddr;
@property (weak) IBOutlet NSButton *mButtonWriteBdAddr;
@property (weak) IBOutlet NSTextField *mLabelSensorData;
@property (weak) IBOutlet NSButton *mButtonMotoEvent;

@end

