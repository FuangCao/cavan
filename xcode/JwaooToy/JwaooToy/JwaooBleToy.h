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
#import "CavanAccelSensor.h"
#import "JwaooToyCommand.h"
#import "JwaooToyResponse.h"
#import "CavanProgressManager.h"

#define JWAOO_TOY_NAME_DEFAULT          @"JwaooToy"
#define JWAOO_TOY_NAME_K100             @"K100"
#define JWAOO_TOY_NAME_MODEL06          @"MODEL-06"
#define JWAOO_TOY_NAME_MODEL10          @"MODEL-10"

#define JWAOO_TOY_UUID_SERVICE      [CBUUID UUIDWithString:@"1888"]
#define JWAOO_TOY_UUID_COMMAND      [CBUUID UUIDWithString:@"1889"]
#define JWAOO_TOY_UUID_EVENT        [CBUUID UUIDWithString:@"188a"]
#define JWAOO_TOY_UUID_FLASH        [CBUUID UUIDWithString:@"188b"]
#define JWAOO_TOY_UUID_SENSOR       [CBUUID UUIDWithString:@"188c"]
#define JWAOO_TOY_UUID_DEBUG        [CBUUID UUIDWithString:@"188d"]

#define JWAOO_TOY_TIME_FUZZ         0.1
#define JWAOO_TOY_ACCEL_FUZZ        2.0
#define JWAOO_TOY_CAPACITY_FUZZ     6.0

#define JWAOO_TOY_KEY_UP            0
#define JWAOO_TOY_KEY_DOWN          2
#define JWAOO_TOY_KEY_O             1
#define JWAOO_TOY_KEY_MAX           3

#define JWAOO_TOY_LED_BATT          1
#define JWAOO_TOY_LED_BT            2

typedef NS_ENUM(NSUInteger, JwaooToyDeviceId) {
    JWAOO_TOY_DEVICE_ID_COMMON,
    JWAOO_TOY_DEVICE_ID_MODEL06,
    JWAOO_TOY_DEVICE_ID_MODEL10,
};

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
- (void)didMotoStateChanged:(uint8_t)mode
                      speed:(uint8_t)speed;
- (void)didBatteryStateChanged:(uint8_t)state;
							level:(uint8_t)level;
							voltage:(double)voltage;
- (void)didSensorDataReceived:(nonnull NSData *)data;
- (void)didDebugDataReceived:(nonnull NSData *)data;
- (void)didConnectStateChanged:(BOOL)connected;

@end

@interface JwaooToyMotoMode : NSObject {
    uint8_t mMode;
    uint8_t mSpeed;
}

@property uint8_t mode;
@property uint8_t speed;

- (nonnull JwaooToyMotoMode *)initWithMode:(uint8_t)mode
                                 withSpeed:(uint8_t)speed;

@end

@interface JwaooToyBattInfo : NSObject {
    uint8_t mState;
    uint8_t mLevel;
    double mVoltage;
}

@property uint8_t state;
@property uint8_t level;
@property double voltage;

- (nonnull JwaooToyBattInfo *)initWithState:(uint8_t)state
                                 withLevel:(uint8_t)level
                                 withVoltage:(double)voltage;

@end

@interface JwaooBleToy : CavanBleGatt {
    CavanBleChar *mCharCommand;
    CavanBleChar *mCharEvent;
    CavanBleChar *mCharFlash;
    CavanBleChar *mCharSensor;
    CavanBleChar *mCharDebug;
    JwaooToyCommand *mCommand;

    CavanAccelSensor *mSensor;
    id<JwaooBleToyDelegate> mDelegate;

    BOOL mUpgradeBusy;
    uint8_t mFlashCrc;
    JwaooToyDeviceId mDeviceId;
}

@property (readonly, nonnull) CavanAccelSensor *sensor;
@property (readonly) JwaooToyDeviceId deviceId;

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
- (BOOL)setKeyLock:(BOOL)enable;
- (BOOL)setKeyReportEnable:(uint8_t)mask;
- (BOOL)setLedEnable:(uint8_t)index
              enable:(BOOL)enable;
- (BOOL)setMotoEventEnable:(BOOL)enable;
- (nonnull JwaooToyMotoMode *)getMotoMode;
- (BOOL)setMotoMode:(uint8_t)mode
          withSpeed:(uint8_t)speed;
- (BOOL)setBattEventEnable:(BOOL)enable;
- (nonnull JwaooToyBattInfo *)readBattInfo;
@end
