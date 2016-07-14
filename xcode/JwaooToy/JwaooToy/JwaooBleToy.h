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
#import "CavanProgressManager.h"

#define JWAOO_TOY_FLASH_MAGIC       0x00005070
#define JWAOO_TOY_IDENTIFY          @"JwaooToy"

#define JWAOO_TOY_UUID_SERVICE      [CBUUID UUIDWithString:@"1888"]
#define JWAOO_TOY_UUID_COMMAND      [CBUUID UUIDWithString:@"1889"]
#define JWAOO_TOY_UUID_EVENT        [CBUUID UUIDWithString:@"188a"]
#define JWAOO_TOY_UUID_FLASH        [CBUUID UUIDWithString:@"188b"]
#define JWAOO_TOY_UUID_SENSOR       [CBUUID UUIDWithString:@"188c"]

#define JWAOO_TOY_TIME_FUZZ         0.1
#define JWAOO_TOY_VALUE_FUZZ        2.0

enum
{
    JWAOO_TOY_RSP_BOOL,
    JWAOO_TOY_RSP_U8,
    JWAOO_TOY_RSP_U16,
    JWAOO_TOY_RSP_U32,
    JWAOO_TOY_RSP_DATA,
    JWAOO_TOY_RSP_TEXT,
    JWAOO_TOY_CMD_NOOP = 20,
    JWAOO_TOY_CMD_IDENTIFY,
    JWAOO_TOY_CMD_VERSION,
    JWAOO_TOY_CMD_BUILD_DATE,
    JWAOO_TOY_CMD_REBOOT,
    JWAOO_TOY_CMD_SHUTDOWN,
    JWAOO_TOY_CMD_BATT_INFO,
    JWAOO_TOY_CMD_FIND,
    JWAOO_TOY_CMD_FLASH_ID = 50,
    JWAOO_TOY_CMD_FLASH_SIZE,
    JWAOO_TOY_CMD_FLASH_PAGE_SIZE,
    JWAOO_TOY_CMD_FLASH_READ,
    JWAOO_TOY_CMD_FLASH_SEEK,
    JWAOO_TOY_CMD_FLASH_ERASE,
    JWAOO_TOY_CMD_FLASH_WRITE_ENABLE,
    JWAOO_TOY_CMD_FLASH_WRITE_START,
    JWAOO_TOY_CMD_FLASH_WRITE_FINISH,
    JWAOO_TOY_CMD_SENSOR_ENABLE = 70,
    JWAOO_TOY_CMD_SENSOR_SET_DELAY,
    JWAOO_TOY_CMD_MOTO_ENABLE = 80,
    JWAOO_TOY_CMD_MOTO_SET_LEVEL,
};

#pragma pack(1)

struct jwaoo_toy_command {
    uint8_t type;

    union {
        uint32_t value32;
        uint16_t value16;
        uint8_t value8;
        uint8_t data[0];
        char text[0];
    };
};

struct jwaoo_toy_flash_header {
    uint32_t magic;
    uint32_t size;
};

#pragma pack()

@class JwaooBleToy;

@protocol JwaooBleToyDelegate <NSObject>
@required
- (BOOL)doInitialize:(nonnull JwaooBleToy *)bleToy;

@optional
- (void)didEventReceived:(nonnull CavanBleChar *)bleChar;
- (void)didSensorDataReceived:(nonnull CavanBleChar *)bleChar;
- (void)didDepthChanged:(int)depth;
- (void)didFreqChanged:(int)freq;
- (void)didConnectStateChanged:(BOOL)connected;

@end

@interface JwaooBleToy : CavanBleGatt <JwaooToyParserDelegate> {
    CavanBleChar *mCharCommand;
    CavanBleChar *mCharEvent;
    CavanBleChar *mCharFlash;
    CavanBleChar *mCharSensor;

    JwaooToySensor *mSensor;
    JwaooToyParser *mParser;
    id<JwaooBleToyDelegate> mDelegate;

    BOOL mUpgradeBusy;
}

@property (readonly) int freq;
@property (readonly) int depth;

- (nonnull JwaooBleToy *)initWithDelegate:(nullable id<JwaooBleToyDelegate>)delegate;

+ (BOOL)parseResponseBool:(nullable NSData *)response;
+ (uint8_t)parseResponseValue8:(nullable NSData *)response;
+ (uint16_t)parseResponseValue16:(nullable NSData *)response;
+ (uint32_t)parseResponseValue32:(nullable NSData *)response;
+ (nullable NSString *)parseResponseText:(nullable NSData *)response;

- (void)onEventReceived:(nonnull CavanBleChar *)bleChar;
- (void)onSensorDataReceived:(nonnull CavanBleChar *)bleChar;

- (nullable NSData *)sendCommand:(nonnull NSData *)command;
- (nullable NSData *)sendCommand:(nonnull struct jwaoo_toy_command *)command
                 length:(NSUInteger)length;
- (nullable NSData *)sendEmptyCommand:(uint8_t) command;
- (nullable NSData *)sendCommand:(uint8_t)type
             withValue8:(uint8_t)value;
- (nullable NSData *)sendCommand:(uint8_t)type
               withBool:(BOOL)value;
- (nullable NSData *)sendCommand:(uint8_t)type
            withValue16:(uint16_t)value;
- (nullable NSData *)sendCommand:(uint8_t)type
            withValue32:(uint32_t)value;

- (BOOL)sendCommandReadBool:(uint8_t)type;
- (uint8_t)sendCommandReadValue8:(uint8_t)type;
- (uint16_t)sendCommandReadValue16:(uint8_t)type;
- (uint32_t)sendCommandReadReadValue32:(uint8_t)type;
- (nullable NSString *)sendCommandReadText:(uint8_t)type;

- (BOOL)sendCommandReadBool:(uint8_t)type
                   withBool:(BOOL)value;
- (BOOL)sendCommandReadBool:(uint8_t)type
                 withValue8:(uint8_t)value;
- (BOOL)sendCommandReadBool:(uint8_t)type
                withValue16:(uint16_t)value;
- (BOOL)sendCommandReadBool:(uint8_t)type
                withValue32:(uint32_t)value;

- (nullable NSString *)doIdentify;
- (nullable NSString *)readBuildDate;
- (uint32_t)readVersion;
- (BOOL)doReboot;

- (BOOL)setSensorEnable:(BOOL)enable;
- (BOOL)setSensorDelay:(uint32_t)delay;

- (uint32_t)getFlashId;
- (uint32_t)getFlashSize;
- (uint32_t)getFlashPageSize;
- (BOOL)setFlashWriteEnable:(BOOL)enable;
- (BOOL)eraseFlash;
- (BOOL)startFlashUpgrade;
- (BOOL)finishFlashUpgrade;
- (BOOL)writeFlash:(nonnull const void *)data
              size:(int)size
      withProgress:(nullable CavanProgressManager *)progress;
- (BOOL)upgradeFirmware:(nonnull const char *)pathname
           withProgress:(nullable CavanProgressManager *)progress;
@end
