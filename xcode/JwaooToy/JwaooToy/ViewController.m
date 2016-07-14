//
//  ViewController.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/7.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "ViewController.h"
#import "Mpu6050Sensor.h"
#import "AccelFreqParser.h"

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // Do any additional setup after loading the view.
    mBleToy = [[JwaooBleToy alloc] initWithSensor:[Mpu6050Sensor new] withDelegate:self];
    [NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(dataSpeedTimer) userInfo:nil repeats:YES];
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

- (BOOL)doInitialize:(JwaooBleToy *)bleToy {
    NSLog(@"doInitialize");
    return true;
}

- (void)didConnectStateChanged:(BOOL)connected {
    NSLog(@"didConnectStateChanged: connected = %d", connected);
}

- (void)didSensorDataReceived:(nonnull CavanBleChar *)bleChar {
    mCount++;
    mFreq = mBleToy.freq;
    mDepth = mBleToy.depth;
    [self performSelectorOnMainThread:@selector(updateFreqDepth) withObject:nil waitUntilDone:NO];
}

- (void)dataSpeedTimer {
    int count;

    count = mCount;
    mCount = 0;

    [self performSelectorOnMainThread:@selector(updateDataSpeed:) withObject:[NSNumber numberWithInt:count] waitUntilDone:NO];
}

- (IBAction)sendCommandButton:(NSButton *)sender {
    NSLog(@"sendCommandButton");

    NSString *text = [mBleToy doIdentify];
    if (text != nil) {
        NSLog(@"doIdentify = %@", text);
    }

    text = [mBleToy readBuildDate];
    if  (text != nil) {
        NSLog(@"BuildDate = %@", text);
    }

    uint32_t version = [mBleToy readVersion];
    NSLog(@"version = 0x%08x", version);
}

- (IBAction)sensorEnableButton:(NSButton *)sender {
    NSLog(@"sensorEnableButton");

    if (mSensorEnable) {
        if ([mBleToy setSensorEnable:FALSE]) {
            mSensorEnable = FALSE;
        }
    } else if ([mBleToy setSensorEnable:TRUE]) {
        [mBleToy setSensorDelay:30];
        mSensorEnable = TRUE;
    }

    NSLog(@"mSensorEnable = %d", mSensorEnable);
}

- (void)enableButton:(NSButton *)button {
    if ([NSThread isMainThread]) {
        button.enabled = true;
    } else {
        [self performSelectorOnMainThread:@selector(enableButton:) withObject:button waitUntilDone:TRUE];
    }
}

- (void)disableButton:(NSButton *)button {
    if ([NSThread isMainThread]) {
        button.enabled = false;
    } else {
        [self performSelectorOnMainThread:@selector(disableButton:) withObject:button waitUntilDone:TRUE];
    }
}

- (void)updateUI:(NSNumber *)enable {
    if ([NSThread isMainThread]) {
        _mButtonReboot.enabled = enable.boolValue;
        _mButtonSensor.enabled = enable.boolValue;
        _mButtonUpgrade.enabled = enable.boolValue;
        _mButtonDisconnect.enabled = enable.boolValue;
        _mButtonSensCommand.enabled = enable.boolValue;
    } else {
        [self performSelectorOnMainThread:@selector(updateUI:) withObject:enable waitUntilDone:NO];
    }
}

- (void)updateDataSpeed:(NSNumber *)count {
    _mLabelDataCount.intValue = count.intValue;

    if (count.intValue > 0) {
        _mLabelDataSpeed.doubleValue = 1000.0 / count.intValue;
    } else {
        _mLabelDataSpeed.stringValue = @"-";
    }
}

- (void)updateFreqDepth {
    _mLabelFreq.intValue = mFreq;
    _mLabelDepth.intValue = mDepth;
}

- (void)didProgressUpdated:(NSNumber *)progress {
    if ([NSThread isMainThread]) {
        _mProgressBar.doubleValue = progress.intValue;
    } else {
        [self performSelectorOnMainThread:@selector(didProgressUpdated:) withObject:progress waitUntilDone:NO];
    }
}

- (void)upgradeThread:(id)data {
    [self updateUI:[NSNumber numberWithBool:FALSE]];

    CavanProgressManager *progress = [[CavanProgressManager alloc] initWithDelegate:self];
    if ([mBleToy upgradeFirmware:"/host/tmp/jwaoo-toy.hex" withProgress:progress]) {
        NSLog(@"upgrade successfull");
    } else {
        NSLog(@"upgrade failed");
    }

    [self updateUI:[NSNumber numberWithBool:TRUE]];
}

- (IBAction)upgradeButton:(NSButton *)sender {
    NSLog(@"upgradeButton");
    _mProgressBar.minValue = 0;
    _mProgressBar.maxValue = 100;
    [NSThread detachNewThreadSelector:@selector(upgradeThread:) toTarget:self withObject:sender];
}

- (IBAction)rebootButton:(NSButton *)sender {
    if ([mBleToy doReboot]) {
        NSLog(@"reboot successfull");
    } else {
        NSLog(@"reboot failed");
    }
}

- (IBAction)disconnectButton:(NSButton *)sender {
    [mBleToy disconnect];
}

@end
