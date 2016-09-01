//
//  JwaooToy.h
//  TestBle
//
//  Created by 曹福昂 on 16/7/7.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CavanBleChar.h"
#import "CavanBleGatt.h"
#import "CavanHexFile.h"
#import "JwaooToySensor.h"
#import "JwaooToyParser.h"
#import "JwaooToyCommand.h"
#import "JwaooToyResponse.h"
#import "CavanProgressManager.h"

#define JWAOO_TOY_IDENTIFY          @"JwaooToy"

#define JWAOO_TOY_UUID_SERVICE      [CBUUID UUIDWithString:@"1888"]
#define JWAOO_TOY_UUID_COMMAND      [CBUUID UUIDWithString:@"1889"]
#define JWAOO_TOY_UUID_EVENT        [CBUUID UUIDWithString:@"188a"]
#define JWAOO_TOY_UUID_FLASH        [CBUUID UUIDWithString:@"188b"]
#define JWAOO_TOY_UUID_SENSOR       [CBUUID UUIDWithString:@"188c"]
#define JWAOO_TOY_UUID_DEBUG        [CBUUID UUIDWithString:@"188d"]

#define JWAOO_TOY_TIME_FUZZ         0.1
#define JWAOO_TOY_ACCEL_FUZZ        2.0
#define JWAOO_TOY_CAPACITY_FUZZ     8.0

@class JwaooBleToy;

@protocol JwaooBleToyDelegate <NSObject>
@required
- (BOOL)doInitialize:(nonnull JwaooBleToy *)bleToy;

@optional
- (void)didKeyStateChanged:(uint8_t)code
                     value:(uint8_t)value;
- (void)didKeyClicked:(uint8_t)code
                count:(uint8_t)count;
- (void)didKeyLongClicked:(uint8_t)code;
- (void)didSensorDataReceived:(nonnull NSData *)data;
- (void)didDebugDataReceived:(nonnull NSData *)data;
- (void)didConnectStateChanged:(BOOL)connected;

@end

@interface JwaooBleToy : CavanBleGatt {
    CavanBleChar *mCharCommand;
    CavanBleChar *mCharEvent;
    CavanBleChar *mCharFlash;
    CavanBleChar *mCharSensor;
    CavanBleChar *mCharDebug;
    JwaooToyCommand *mCommand;

    JwaooToySensor *mSensor;
    JwaooToyParser *mParser;
    id<JwaooBleToyDelegate> mDelegate;

    BOOL mUpgradeBusy;
    uint8_t mFlashCrc;
}

@property (readonly) double freq;
@property (readonly) double depth;

- (nonnull JwaooBleToy *)initWithDelegate:(nullable id<JwaooBleToyDelegate>)delegate;

- (void)onEventReceived:(nonnull NSData *)data;
- (void)onSensorDataReceived:(nonnull NSData *)data;
- (void)onDebugDataReceived:(nonnull NSData *)data;

- (nullable NSString *)doIdentify;
- (nullable NSString *)readBuildDate;
- (uint32_t)readVersion;
- (BOOL)doReboot;

- (BOOL)setSensorEnable:(BOOL)enable;
- (BOOL)setSensorEnable:(BOOL)enable
              withDelay:(uint32_t)delay;

- (uint32_t)getFlashId;
- (uint32_t)getFlashSize;
- (uint32_t)getFlashPageSize;
- (BOOL)setFlashWriteEnable:(BOOL)enable;
- (BOOL)eraseFlash;
- (BOOL)startFlashUpgrade;
- (BOOL)finishFlashUpgrade:(uint16_t)length;
- (BOOL)writeFlash:(nonnull const void *)data
              size:(int)size
      withProgress:(nullable CavanProgressManager *)progress;
- (BOOL)upgradeFirmware:(nonnull const char *)pathname
           withProgress:(nullable CavanProgressManager *)progress;
- (nullable NSData *)readBdAddress;
- (nullable NSString *)readBdAddressString;
- (BOOL)writeBdAddressWithBytes:(nonnull const uint8_t *)bytes;
- (BOOL)writeBdAddressWithString:(nonnull NSString *)addr;
- (BOOL)setKeyClickEnable:(BOOL)enable;
- (BOOL)setKeyLongClickEnable:(BOOL)enable;
- (BOOL)setKeyLongClickEnable:(BOOL)enable
                 withDelay:(uint16_t)delay;
- (BOOL)setKeyMultiClickEnable:(BOOL)enable;
- (BOOL)setKeyMultiClickEnable:(BOOL)enable
                  withDelay:(uint16_t)delay;

@end
