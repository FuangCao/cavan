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

@interface JwaooBleToyEventDelegate : NSObject <CavanBleCharDelegate> {
    ViewController *mController;
}

- (JwaooBleToyEventDelegate *)initWithViewController:(ViewController *)controller;

@end

@implementation JwaooBleToyEventDelegate

- (JwaooBleToyEventDelegate *)initWithViewController:(ViewController *)controller {
    if (self = [super init]) {
        mController = controller;
    }

    return self;
}

- (void)didNotifyForCharacteristic:(nonnull CavanBleChar *)characteristic {
    NSLog(@"JwaooBleToyEventDelegate: didNotifyForCharacteristic");
}

@end

@interface JwaooBleToySensorDelegate : NSObject <CavanBleCharDelegate, AccelFreqParserDelegate> {
    ViewController *mController;
    AccelFreqParser *mParser;
}

- (JwaooBleToySensorDelegate *)initWithViewController:(ViewController *)controller;

@end

@implementation JwaooBleToySensorDelegate

- (JwaooBleToySensorDelegate *)initWithViewController:(ViewController *)controller {
    if (self = [super init]) {
        mController = controller;
        mParser = [[AccelFreqParser alloc] initWithValueFuzz:2.0 withTimeFuzz:0.06 withDelegate:self];
    }

    return self;
}

- (void)didDepthChanged:(int)depth {
    NSLog(@"depth = %d", depth);
}

- (void)didFreqChanged:(int)freq {
    NSLog(@"freq = %d", freq);
}

- (void)didNotifyForCharacteristic:(nonnull CavanBleChar *)characteristic {
    // NSLog(@"JwaooBleToySensorDelegate: didNotifyForCharacteristic");
    [mParser putBytes:[characteristic getData].bytes];
}

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // Do any additional setup after loading the view.
    mBleToy = [[JwaooBleToy alloc] initWithName:@"JwaooToy" uuid:JWAOO_TOY_UUID_SERVICE];
    [mBleToy setEventDelegate:[[JwaooBleToyEventDelegate alloc] initWithViewController:self]];
    [mBleToy setSensorDelegate:[[JwaooBleToySensorDelegate alloc] initWithViewController:self]];
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
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
        [mBleToy setSensorDelay:10];
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
        [self performSelectorOnMainThread:@selector(updateUI:) withObject:enable waitUntilDone:YES];
    }
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
