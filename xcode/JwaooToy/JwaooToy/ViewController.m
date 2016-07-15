//
//  ViewController.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/7.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "ViewController.h"

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // Do any additional setup after loading the view.
    mBleToy = [[JwaooBleToy alloc] initWithDelegate:self];
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

- (void)didDepthChanged:(int)depth {
    // NSLog(@"didDepthChanged: depth = %d", depth);
}

- (void)didFreqChanged:(int)freq {
    // NSLog(@"didFreqChanged: freq = %d", freq);
}

- (void)dataSpeedTimer {
    int count;

    count = mCount;
    mCount = 0;

    [self performSelectorOnMainThread:@selector(updateDataSpeed:) withObject:[NSNumber numberWithInt:count] waitUntilDone:NO];
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

- (void)onProgressUpdated:(NSNumber *)progress {
    if ([NSThread isMainThread]) {
        _mProgressBar.doubleValue = progress.intValue;
    } else {
        [self performSelectorOnMainThread:@selector(onProgressUpdated:) withObject:progress waitUntilDone:NO];
    }
}

- (void)upgradeThread:(id)data {
    [self updateUI:[NSNumber numberWithBool:FALSE]];

    CavanProgressManager *progress = [[CavanProgressManager alloc] initWithSelector:@selector(onProgressUpdated:) withTarget:self];
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

- (IBAction)buttonReadBdAddr:(NSButton *)sender {
    NSData *addr = [mBleToy readBdAddress];
    if (addr != nil) {
        const uint8_t *bytes = addr.bytes;

        _mTextFieldBdAddr.stringValue = [NSString stringWithFormat:@"%02x:%02x:%02x:%02x:%02x:%02x", bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]];
    } else {
        NSLog(@"Failed to readBdAddress");
    }
}

- (IBAction)buttonWriteBdAddr:(NSButton *)sender {
    const char *text = [_mTextFieldBdAddr.stringValue cStringUsingEncoding:NSASCIIStringEncoding];

    int values[6];

    if (sscanf(text, "%02x:%02x:%02x:%02x:%02x:%02x", values, values + 1, values + 2, values + 3, values + 4, values + 5) == 6) {
        uint8_t bytes[6];

        for (int i = 0; i < 6; i++) {
            bytes[i] = values[i];
        }

        if ([mBleToy writeBdAddress:bytes]) {
            NSLog(@"writeBdAddress successfull");
        } else {
            NSLog(@"Failed to writeBdAddress");
        }
    } else {
        NSLog(@"Invalid format");
    }
}

@end
