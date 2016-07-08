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
        mParser = [[AccelFreqParser alloc] initWithValueFuzz:2.0 withTimeFuzz:0.1 withDelegate:self];
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
    mBleToy = [[JwaooBleToy alloc] initWithName:@"JwaooToy" uuid:nil];
    [mBleToy setEventDelegate:[[JwaooBleToyEventDelegate alloc] initWithViewController:self]];
    [mBleToy setSensorDelegate:[[JwaooBleToySensorDelegate alloc] initWithViewController:self]];
    [mBleToy startScan];
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

@end
