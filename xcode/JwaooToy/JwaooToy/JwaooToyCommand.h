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
	JWAOO_TOY_UUID_SVC = 0x1888,
	JWAOO_TOY_UUID_COMMAND,
	JWAOO_TOY_UUID_EVENT,
	JWAOO_TOY_UUID_FLASH,
	JWAOO_TOY_UUID_SENSOR,
	JWAOO_TOY_UUID_DEBUG,
	JWAOO_TOY_UUID_MAX
};

enum
{
	JWAOO_TOY_ATTR_SVC,

	JWAOO_TOY_ATTR_COMMAND_CHAR,
	JWAOO_TOY_ATTR_COMMAND_DATA,

	JWAOO_TOY_ATTR_EVENT_CHAR,
	JWAOO_TOY_ATTR_EVENT_DATA,
	JWAOO_TOY_ATTR_EVENT_CFG,

	JWAOO_TOY_ATTR_FLASH_CHAR,
	JWAOO_TOY_ATTR_FLASH_DATA,

	JWAOO_TOY_ATTR_SENSOR_CHAR,
	JWAOO_TOY_ATTR_SENSOR_DATA,
	JWAOO_TOY_ATTR_SENSOR_CFG,

	JWAOO_TOY_ATTR_DEBUG_CHAR,
	JWAOO_TOY_ATTR_DEBUG_DATA,
	JWAOO_TOY_ATTR_DEBUG_CFG,

	JWAOO_TOY_ATTR_COUNT,
};

enum
{
	JWAOO_TOY_RSP_BOOL,
	JWAOO_TOY_RSP_U8,
	JWAOO_TOY_RSP_U16,
	JWAOO_TOY_RSP_U32,
	JWAOO_TOY_RSP_DATA,
	JWAOO_TOY_RSP_TEXT,
};

enum
{
	JWAOO_TOY_CMD_NOOP,
	JWAOO_TOY_CMD_IDENTIFY,
	JWAOO_TOY_CMD_VERSION,
	JWAOO_TOY_CMD_BUILD_DATE,
	JWAOO_TOY_CMD_REBOOT,
	JWAOO_TOY_CMD_SHUTDOWN,
	JWAOO_TOY_CMD_I2C_RW,
	JWAOO_TOY_CMD_SUSPEND_DELAY,
	JWAOO_TOY_CMD_APP_DATA,
	JWAOO_TOY_CMD_APP_SETTINGS,
	JWAOO_TOY_CMD_FLASH_ID = 30,
	JWAOO_TOY_CMD_FLASH_SIZE,
	JWAOO_TOY_CMD_FLASH_PAGE_SIZE,
	JWAOO_TOY_CMD_FLASH_READ,
	JWAOO_TOY_CMD_FLASH_SEEK,
	JWAOO_TOY_CMD_FLASH_ERASE,
	JWAOO_TOY_CMD_FLASH_STATE,
	JWAOO_TOY_CMD_FLASH_WRITE_ENABLE,
	JWAOO_TOY_CMD_FLASH_WRITE_START,
	JWAOO_TOY_CMD_FLASH_WRITE_FINISH,
	JWAOO_TOY_CMD_FLASH_READ_BD_ADDR,
	JWAOO_TOY_CMD_FLASH_WRITE_BD_ADDR,
	JWAOO_TOY_CMD_FACTORY_ENABLE = 50,
	JWAOO_TOY_CMD_LED_ENABLE,
	JWAOO_TOY_CMD_READ_TEST_RESULT,
	JWAOO_TOY_CMD_WRITE_TEST_RESULT,
	JWAOO_TOY_CMD_BATT_INFO = 60,
	JWAOO_TOY_CMD_BATT_EVENT_ENABLE,
	JWAOO_TOY_CMD_BATT_SHUTDOWN_VOLTAGE,
	JWAOO_TOY_CMD_SENSOR_ENABLE = 70,
	JWAOO_TOY_CMD_MOTO_SET_MODE = 80,
	JWAOO_TOY_CMD_MOTO_GET_MODE,
	JWAOO_TOY_CMD_MOTO_EVENT_ENABLE,
	JWAOO_TOY_CMD_MOTO_SPEED_TABLE,
	JWAOO_TOY_CMD_KEY_CLICK_ENABLE = 90,
	JWAOO_TOY_CMD_KEY_LONG_CLICK_ENABLE,
	JWAOO_TOY_CMD_KEY_MULTI_CLICK_ENABLE,
	JWAOO_TOY_CMD_KEY_LOCK,
	JWAOO_TOY_CMD_KEY_REPORT_ENABLE,
	JWAOO_TOY_CMD_GPIO_GET = 100,
	JWAOO_TOY_CMD_GPIO_SET,
	JWAOO_TOY_CMD_GPIO_CFG,
};

enum
{
	JWAOO_TOY_EVT_NOOP,
	JWAOO_TOY_EVT_BATT_INFO,
	JWAOO_TOY_EVT_KEY_STATE,
	JWAOO_TOY_EVT_KEY_CLICK,
	JWAOO_TOY_EVT_KEY_LONG_CLICK,
	JWAOO_TOY_EVT_UPGRADE_COMPLETE,
	JWAOO_TOY_EVT_MOTO_STATE_CHANGED,
};

enum
{
	JWAOO_TOY_KEY_VALUE_UP,
	JWAOO_TOY_KEY_VALUE_DOWN,
	JWAOO_TOY_KEY_VALUE_REPEAT,
	JWAOO_TOY_KEY_VALUE_LONG,
};

enum
{
	JWAOO_TOY_BATTERY_NORMAL,
	JWAOO_TOY_BATTERY_LOW,
	JWAOO_TOY_BATTERY_CHARGING,
	JWAOO_TOY_BATTERY_FULL,
};

#pragma pack(1)

struct jwaoo_toy_command
{
	uint8_t type;

	union {
		char text[1];
		uint8_t bytes[1];
		uint16_t words[1];
		uint32_t dwords[1];
		uint8_t value8;
		uint16_t value16;
		uint32_t value32;

		struct {
			uint8_t crc;
			uint16_t length;
		} upgrade;

		struct {
			uint8_t value;

			union {
				uint8_t delay8;
				uint16_t delay16;
				uint32_t delay32;
			};
		} enable;

		struct {
			uint8_t index;
			uint8_t value;
		} node;

		struct {
			uint8_t slave;
			uint8_t rdlen;
			uint8_t data[];
		} i2c;

		struct {
			uint8_t mode;
			uint8_t level;
		} moto;

		struct {
			uint8_t index;
			uint8_t data[9];
		} speed_table;

		struct {
			uint8_t index;
			uint8_t enable;
		} led;

		struct {
			uint8_t state;
			uint8_t level;
			uint16_t voltage;
		} battery;

		struct {
			uint8_t port;
			uint8_t pin;
			uint8_t value;
		} gpio;

		struct {
			uint8_t port;
			uint8_t pin;
			uint8_t mode;
			uint8_t function;
			uint8_t high;
		} gpio_config;

		struct {
			uint16_t valid;
			uint16_t result;
		} test_result;
	};
};

struct jwaoo_toy_response
{
	uint8_t command;
	uint8_t type;

	union {
		char text[1];
		uint8_t bytes[1];
		uint16_t words[1];
		uint32_t dwords[1];
		uint8_t value8;
		uint16_t value16;
		uint32_t value32;

		struct {
			uint16_t valid;
			uint16_t result;
		} test_result;

		struct {
			uint8_t state;
			uint8_t level;
			uint16_t voltage;
		} battery;

		struct {
			uint8_t mode;
			uint8_t level;
		} moto;
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
