//
//  ViewController.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/7.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "ViewController.h"
#import "CavanCountedArray.h"
#import "CavanWaveArray.h"

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

    if (![bleToy setSensorEnable:_mButtonSensor.state withDelay:30]) {
        NSLog(@"Failed to setSensorEnable");
        return false;
    }

    if (![bleToy setKeyClickEnable:_mButtonClick.state]) {
        NSLog(@"Failed to setKeyClickEnable");
        return false;
    }

    if (![bleToy setKeyLongClickEnable:_mButtonLongClick.state]) {
        NSLog(@"Failed to setKeyLongClickEnable");
        return false;
    }

    if (![bleToy setKeyMultiClickEnable:_mButtonMultiClick.state]) {
        NSLog(@"Failed to setKeyMultiClickEnable");
        return false;
    }

    if (![bleToy setBattEventEnable:_mButtonBattInfo.state]) {
        NSLog(@"Failed to setBattEventEnable");
        return false;
    }

    [bleToy setKeyReportEnable:0xFF];

    JwaooToyBattInfo *info = [bleToy readBattInfo];
    if (info == nil) {
        NSLog(@"Failed to readBattInfo");
        return false;
    }

    NSLog(@"battery info: %@", info);

    return true;
}

- (void)didConnectStateChanged:(BOOL)connected {
    NSLog(@"didConnectStateChanged: connected = %d", connected);
}

- (void)didSensorDataReceived:(nonnull NSData *)data {
    mCount++;
    [self performSelectorOnMainThread:@selector(updateSensorData) withObject:nil waitUntilDone:NO];
}

- (void)didBatteryStateChanged:(uint8_t)state level:(uint8_t)level voltage:(double)voltage {
    NSLog(@"didBatteryStateChanged: state = %d, level = %d, voltage = %3.2lf", state, level, voltage);
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
        _mButtonClick.enabled = enable.boolValue;
        _mButtonLongClick.enabled = enable.boolValue;
        _mButtonMultiClick.enabled = enable.boolValue;
        _mButtonReadBdAddr.enabled = enable.boolValue;
        _mButtonWriteBdAddr.enabled = enable.boolValue;
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

- (void)updateSensorData {
    _mLabelSensorData.stringValue = [mBleToy.sensor description];
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
    if (text == nil) {
        NSLog(@"Failed to doIdentify");
        return;
    }

    NSLog(@"doIdentify = %@", text);

    text = [mBleToy readBuildDate];
    if  (text == nil) {
        NSLog(@"Failed to readBuildDate");
        return;
    }

    NSLog(@"BuildDate = %@", text);

    uint32_t version = [mBleToy readVersion];
    NSLog(@"version = 0x%08x", version);
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

- (IBAction)connectButton:(NSButton *)sender {
#if 0
    NSUUID *uuid = [[NSUUID alloc] initWithUUIDString:@"6BB9E94D-75AF-40DF-9597-07D067636AF2"];
    [mBleToy connectByIdentify:uuid];
#else
    [mBleToy connect];
#endif
}

- (IBAction)buttonReadBdAddr:(NSButton *)sender {
    NSString *addr = [mBleToy readBdAddressString];
    if (addr != nil) {
        _mTextFieldBdAddr.stringValue = addr;
    }
}

- (IBAction)buttonWriteBdAddr:(NSButton *)sender {
    if ([mBleToy writeBdAddressWithString:_mTextFieldBdAddr.stringValue]) {
        NSLog(@"writeBdAddress successfull");
    } else {
        NSLog(@"Failed to writeBdAddress");
    }
}

- (IBAction)sensorEnableButton:(NSButton *)sender {
    [mBleToy setSensorEnable:sender.state withDelay:30];
}

- (IBAction)buttonClick:(NSButton *)sender {
    [mBleToy setKeyClickEnable:sender.state];
}

- (IBAction)buttonLongClick:(NSButton *)sender {
    [mBleToy setKeyLongClickEnable:sender.state];
}

- (IBAction)buttonMultiClick:(NSButton *)sender {
    [mBleToy setKeyMultiClickEnable:sender.state];
}

- (IBAction)buttonMotoEventClick:(NSButton *)sender {
    [mBleToy setMotoEventEnable:sender.state];
}

- (IBAction)buttonKeyLockClick:(NSButton *)sender {
    [mBleToy setKeyLock:sender.state];
}

- (IBAction)buttonBattInfoClick:(NSButton *)sender {
    [mBleToy setBattEventEnable:sender.state];
}

- (IBAction)buttonMotoCtrlClick:(NSButton *)sender {
    [mBleToy setMotoMode:_mTextFieldMotoMode.intValue withSpeed:_mTextFieldMotoLevel.intValue];
}

@end
