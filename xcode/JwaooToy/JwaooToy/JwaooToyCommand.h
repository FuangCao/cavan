//
//  JwaooToyCommand.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/19.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CavanBleChar.h"

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
    JWAOO_TOY_CMD_I2C_RW,
    JWAOO_TOY_CMD_FLASH_ID = 50,
    JWAOO_TOY_CMD_FLASH_SIZE,
    JWAOO_TOY_CMD_FLASH_PAGE_SIZE,
    JWAOO_TOY_CMD_FLASH_READ,
    JWAOO_TOY_CMD_FLASH_SEEK,
    JWAOO_TOY_CMD_FLASH_ERASE,
    JWAOO_TOY_CMD_FLASH_WRITE_ENABLE,
    JWAOO_TOY_CMD_FLASH_WRITE_START,
    JWAOO_TOY_CMD_FLASH_WRITE_FINISH,
    JWAOO_TOY_CMD_FLASH_READ_BD_ADDR,
    JWAOO_TOY_CMD_FLASH_WRITE_BD_ADDR,
    JWAOO_TOY_CMD_SENSOR_ENABLE = 70,
    JWAOO_TOY_CMD_MOTO_ENABLE = 80,
    JWAOO_TOY_CMD_KEY_CLICK_ENABLE = 90,
    JWAOO_TOY_CMD_KEY_LONG_CLICK_ENABLE,
    JWAOO_TOY_CMD_KEY_MULTI_CLICK_ENABLE,
};

enum
{
    JWAOO_TOY_EVT_NOOP,
    JWAOO_TOY_EVT_BATT_INFO,
    JWAOO_TOY_EVT_KEY_STATE,
    JWAOO_TOY_EVT_KEY_CLICK,
    JWAOO_TOY_EVT_KEY_LONG_CLICK,
};


#pragma pack(1)

struct jwaoo_toy_command
{
    uint8_t type;

    union {
        char text[1];
        uint8_t bytes[1];
        uint8_t value8;
        uint16_t value16;
        uint32_t value32;

        struct {
            uint8_t crc;
            uint16_t length;
        };

        struct {
            uint8_t enable;

            union {
                uint8_t delay8;
                uint16_t delay16;
                uint32_t delay32;
            };
        };
    };
};

struct jwaoo_toy_response
{
    uint8_t command;
    uint8_t type;

    union {
        char text[1];
        uint8_t bytes[1];
        uint8_t value8;
        uint16_t value16;
        uint32_t value32;
    };
};

#pragma pack()

@class JwaooToyResponse;

@interface JwaooToyCommand : NSObject {
    CavanBleChar *mChar;
}

- (nullable JwaooToyCommand *)initWithBleChar:(nonnull CavanBleChar *)bleChar;
- (nullable JwaooToyResponse *)send:(nonnull NSData *)command;
- (nullable JwaooToyResponse *)sendWithBytes:(nonnull const void *)bytes
                    length:(NSUInteger)length;
- (nullable JwaooToyResponse *)sendWithType:(uint8_t)type;
- (nullable JwaooToyResponse *)sendWithType:(uint8_t)type
                          withBytes:(nonnull const void *)bytes
                            length:(NSUInteger)length;
- (nullable JwaooToyResponse *)sendWithType:(uint8_t)type
                          withBool:(BOOL)value;
- (nullable JwaooToyResponse *)sendWithType:(uint8_t)type
                          withBool:(BOOL)enable
                       withDelay16:(uint16_t)delay;
- (nullable JwaooToyResponse *)sendWithType:(uint8_t)type
                          withBool:(BOOL)enable
                       withDelay32:(uint32_t)delay;
- (nullable JwaooToyResponse *)sendWithType:(uint8_t)type
                        withValue8:(uint8_t)value;
- (nullable JwaooToyResponse *)sendWithType:(uint8_t)type
                       withValue16:(uint16_t)value;
- (nullable JwaooToyResponse *)sendWithType:(uint8_t)type
                      withValue32:(uint32_t)value;

- (BOOL)readBoolWithBytes:(nonnull const void *)bytes
                   length:(NSUInteger)length;
- (BOOL)readBoolWithType:(uint8_t)type;
- (uint8_t)readValueWithType8:(uint8_t)type
                     defValue:(uint8_t)defValue;
- (uint8_t)readValueWithType8:(uint8_t)type;
- (uint16_t)readValueWithType16:(uint8_t)type
                       defValue:(uint16_t)defValue;
- (uint16_t)readValueWithType16:(uint8_t)type;
- (uint32_t)readValueWithType32:(uint8_t)type
                       defValue:(uint32_t)defValue;
- (uint32_t)readValueWithType32:(uint8_t)type;
- (nullable NSString *)readTextWithType:(uint8_t)type;
- (nullable NSData *)readDataWithType:(uint8_t)type;
- (BOOL)readBoolWithType:(uint8_t)type
                withBool:(BOOL)value;
- (BOOL)readBoolWithType:(uint8_t)type
              withValue8:(uint8_t)value;
- (BOOL)readBoolWithType:(uint8_t)type
             withValue16:(uint16_t)value;
- (BOOL)readBoolWithType:(uint8_t)type
             withValue32:(uint32_t)value;
- (BOOL)readBoolWithType:(uint8_t)type
                withBytes:(nonnull const void *)bytes
                   length:(NSUInteger)length;
- (BOOL)readBoolWithType:(uint8_t)type
                withText:(nonnull NSString *)text;
- (BOOL)readBoolWithType:(uint8_t)type
                withBool:(BOOL)enable
             withDelay16:(uint16_t)delay;
- (BOOL)readBoolWithType:(uint8_t)type
                withBool:(BOOL)enable
             withDelay32:(uint32_t)delay;
@end
