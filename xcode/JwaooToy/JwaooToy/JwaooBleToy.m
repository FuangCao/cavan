//
//  JwaooToy.m
//  TestBle
//
//  Created by 曹福昂 on 16/7/7.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "JwaooBleToy.h"
#import "CavanHexFile.h"
#import "JwaooToySensorDefault.h"
#import "JwaooToySensorModel6.h"

@implementation JwaooToyMotoMode

@synthesize mode = mMode;
@synthesize speed = mSpeed;

- (JwaooToyMotoMode *)initWithMode:(uint8_t)mode
                         withSpeed:(uint8_t)speed {
    if (self = [super init]) {
        mMode = mode;
        mSpeed = speed;
    }

    return self;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"mode: %d, speed: %d", mMode, mSpeed];
}

@end

@implementation JwaooBleToy

@synthesize sensor = mSensor;

- (JwaooBleToy *)initWithDelegate:(id<JwaooBleToyDelegate>)delegate {
    if (self = [super initWithNames:@[@"JwaooToy", @"SenseTube"] uuid:JWAOO_TOY_UUID_SERVICE]) {
        mDelegate = delegate;
    }

    return self;
}

- (void)onEventReceived:(NSData *)event {
    NSUInteger length = event.length;

    if (length > 0) {
        const uint8_t *bytes = event.bytes;

        switch (bytes[0]) {
            case JWAOO_TOY_EVT_KEY_STATE:
                if (length < 3) {
                    break;
                }

                if ([mDelegate respondsToSelector:@selector(didKeyStateChanged:value:)]) {
                    [mDelegate didKeyStateChanged:bytes[1] value:bytes[2]];
                } else {
                    NSLog(@"KeyStateChanged: code = %d, value = %d", bytes[1], bytes[2]);
                }
                break;

            case JWAOO_TOY_EVT_KEY_CLICK:
                if (length < 3) {
                    break;
                }

                if ([mDelegate respondsToSelector:@selector(didKeyClicked:count:)]) {
                    [mDelegate didKeyClicked:bytes[1] count:bytes[2]];
                } else {
                    NSLog(@"KeyClicked: code = %d, count = %d", bytes[1], bytes[2]);
                }
                break;

            case JWAOO_TOY_EVT_KEY_LONG_CLICK:
                if (length < 2) {
                    break;
                }

                if ([mDelegate respondsToSelector:@selector(didKeyLongClicked:)]) {
                    [mDelegate didKeyLongClicked:bytes[1]];
                } else {
                    NSLog(@"KeyLongClicked: code = %d", bytes[1]);
                }
                break;

            case JWAOO_TOY_EVT_MOTO_STATE_CHANGED:
                if (length < 3) {
                    break;
                }

                if ([mDelegate respondsToSelector:@selector(didMotoStateChanged:speed:)]) {
                    [mDelegate didMotoStateChanged:bytes[1] speed:bytes[2]];
                } else {
                    NSLog(@"didMotoStateChanged: mode = %d, speed = %d", bytes[1], bytes[2]);
                }
                break;

            default:
                NSLog(@"unknown event%d, length = %lu", bytes[0], (unsigned long)length);
        }
    }
}

- (void)onSensorDataReceived:(NSData *)data {
    [mSensor putBytes:data.bytes];

    if ([mDelegate respondsToSelector:@selector(didSensorDataReceived:)]) {
        [mDelegate didSensorDataReceived:data];
    }
}

- (void)onDebugDataReceived:(NSData *)data {
    if ([mDelegate respondsToSelector:@selector(didDebugDataReceived:)]) {
        [mDelegate didDebugDataReceived:data];
    } else {
        NSLog(@"Debug: %@", [NSString stringWithCString:data.bytes encoding:NSASCIIStringEncoding]);
    }
}

- (void)onConnectStateChanged:(BOOL)connected {
    if ([mDelegate respondsToSelector:@selector(didConnectStateChanged:)]) {
        [mDelegate didConnectStateChanged:connected];
    } else {
        [self onConnectStateChanged:connected];
    }
}

- (BOOL)doInitialize {
    if (![mService.UUID isEqualTo:JWAOO_TOY_UUID_SERVICE]) {
        NSLog(@"Invalid service uuid: %@", mService.UUID);
        return false;
    }

    mCharCommand = nil;
    mCharEvent = nil;
    mCharFlash = nil;
    mCharSensor = nil;

    for (CBCharacteristic *characteristic in mService.characteristics) {
        if ([characteristic.UUID isEqual:JWAOO_TOY_UUID_COMMAND]) {
            mCharCommand = [self createBleChar:characteristic];
            NSLog(@"mCharCommand = %@", characteristic.UUID);
        } else if ([characteristic.UUID isEqual:JWAOO_TOY_UUID_EVENT]) {
            mCharEvent = [self createBleChar:characteristic];
            [mCharEvent enableNotifyWithSelector:@selector(onEventReceived:) withTarget:self];
            NSLog(@"mCharEvent = %@", characteristic.UUID);
        } else if ([characteristic.UUID isEqual:JWAOO_TOY_UUID_FLASH]) {
            mCharFlash = [self createBleChar:characteristic];
            NSLog(@"mCharFlash = %@", characteristic.UUID);
        } else if ([characteristic.UUID isEqual:JWAOO_TOY_UUID_SENSOR]) {
            mCharSensor = [self createBleChar:characteristic];
            [mCharSensor enableNotifyWithSelector:@selector(onSensorDataReceived:) withTarget:self];
            NSLog(@"mCharSensor = %@", characteristic.UUID);
        } else if ([characteristic.UUID isEqual:JWAOO_TOY_UUID_DEBUG]) {
            mCharDebug = [self createBleChar:characteristic];
            [mCharDebug enableNotifyWithSelector:@selector(onDebugDataReceived:) withTarget:self];
            NSLog(@"mCharSensor = %@", characteristic.UUID);
        } else {
            NSLog(@"Unknown characteristic = %@", characteristic.UUID);
            return false;
        }
    }

    if (mCharCommand == nil) {
        NSLog(@"Command characteristic not found: uuid = %@", JWAOO_TOY_UUID_COMMAND);
        return false;
    }

    if (mCharEvent == nil) {
        NSLog(@"Event characteristic not found: uuid = %@", JWAOO_TOY_UUID_EVENT);
        return false;
    }

    if (mCharFlash == nil) {
        NSLog(@"Flash characteristic not found: uuid = %@", JWAOO_TOY_UUID_FLASH);
        return false;
    }

    if (mCharSensor == nil) {
        NSLog(@"Sensor characteristic not found: uuid = %@", JWAOO_TOY_UUID_SENSOR);
        return false;
    }

    mCommand = [[JwaooToyCommand alloc] initWithBleChar:mCharCommand];

    NSString *identify = [self doIdentify];
    if (identify == nil) {
        NSLog(@"Failed to doIdentify");
        return false;
    }

    NSLog(@"identify = %@", identify);

    if ([identify isEqualToString:JWAOO_TOY_ID_DEFAULT]) {
        mSensor = [JwaooToySensorDefault new];
    } else if ([identify isEqualToString:JWAOO_TOY_ID_K100]) {
        mSensor = [JwaooToySensorDefault new];
    } else if ([identify isEqualToString:JWAOO_TOY_ID_MODEL6]) {
        mSensor = [JwaooToySensorModel6 new];
    } else {
        NSLog(@"Invalid identify");
        return false;
    }

    return [mDelegate doInitialize:self];
}

// ================================================================================

- (NSString *)doIdentify {
    return [mCommand readTextWithType:JWAOO_TOY_CMD_IDENTIFY];
}

- (NSString *)readBuildDate {
    return [mCommand readTextWithType:JWAOO_TOY_CMD_BUILD_DATE];
}

- (uint32_t)readVersion {
    return [mCommand readValueWithType32:JWAOO_TOY_CMD_VERSION];
}

- (BOOL)doReboot {
    return [mCommand readBoolWithType:JWAOO_TOY_CMD_REBOOT];
}

- (BOOL)setSensorEnable:(BOOL)enable {
    return [mCommand readBoolWithType:JWAOO_TOY_CMD_SENSOR_ENABLE withBool:enable];
}

- (BOOL)setSensorEnable:(BOOL)enable
              withDelay:(uint32_t)delay {
    return [mCommand readBoolWithType:JWAOO_TOY_CMD_SENSOR_ENABLE withBool:enable withDelay32:delay];
}

- (uint32_t)getFlashId {
    return [mCommand readValueWithType32:JWAOO_TOY_CMD_FLASH_ID];
}

- (uint32_t)getFlashSize {
    return [mCommand readValueWithType32:JWAOO_TOY_CMD_FLASH_SIZE];
}

- (uint32_t)getFlashPageSize {
    return [mCommand readValueWithType32:JWAOO_TOY_CMD_FLASH_PAGE_SIZE];
}

- (BOOL)setFlashWriteEnable:(BOOL)enable {
    return [mCommand readBoolWithType:JWAOO_TOY_CMD_FLASH_WRITE_ENABLE withBool:enable];
}

- (BOOL)eraseFlash {
    return [mCommand readBoolWithType:JWAOO_TOY_CMD_FLASH_ERASE];
}

- (BOOL)startFlashUpgrade {
    return [mCommand readBoolWithType:JWAOO_TOY_CMD_FLASH_WRITE_START];
}

- (BOOL)finishFlashUpgrade:(uint16_t)length {
    uint8_t command[] = { JWAOO_TOY_CMD_FLASH_WRITE_FINISH, mFlashCrc, length & 0xFF, length >> 8 };
    return [mCommand readBoolWithBytes:command length:sizeof(command)];
}

- (BOOL)writeFlash:(const void *)data
              size:(int)size
      withProgress:(CavanProgressManager *)progress {
    if (mCharFlash == nil) {
        return false;
    }

    if ([mCharFlash writeData:data length:size withProgress:progress]) {
        const uint8_t *p, *p_end;

        for (p = data, p_end = p + size; p < p_end; p++) {
            mFlashCrc ^= *p;
        }

        return true;
    }

    return false;
}

- (BOOL)writeFlashHeader:(uint16_t)length {
    length = (length + 7) & (~7);

    uint8_t header[8] = { 0x70, 0x50, 0x00, 0x00, 0x00, 0x00, length >> 8, length & 0xFF };

    return [self writeFlash:header size:sizeof(header) withProgress:nil];
}

- (BOOL)upgradeFirmwareSafe:(nonnull const char *)pathname
               withProgress:(nullable CavanProgressManager *)progress {
    uint32_t flashId = [self getFlashId];
    uint32_t flashSize = [self getFlashSize];
    uint32_t flashPageSize = [self getFlashPageSize];

    [progress setProgressRange:99];

    NSLog(@"ID = 0x%08x, size = %d, page_size = %d", flashId, flashSize, flashPageSize);

    CavanHexFile *file = [[CavanHexFile alloc] initWithPath:pathname mode:nil];
    if (file == nil) {
        return FALSE;
    }

    [progress addProgress];

    NSLog(@"parse bin file: %s", pathname);

    NSData *data = [file readBinData];
    if (data == nil) {
        NSLog(@"Failed to readBinData");
        return FALSE;
    }

    [progress addProgress];

    NSLog(@"length = %ld = 0x%08lx", data.length, data.length);

    NSLog(@"write flash enable");

    if (![self setFlashWriteEnable:true]) {
        NSLog(@"Failed to setWriteEnable true");
        return false;
    }

    [progress addProgress];

    NSLog(@"start upgrade");

    if (![self startFlashUpgrade]) {
        NSLog(@"Failed to startUpgrade");
        return false;
    }

    [progress addProgress];

    NSLog(@"erase flash");

    if (![self eraseFlash]) {
        NSLog(@"Failed to doErase");
        return false;
    }

    [progress addProgress];

    mFlashCrc = 0xFF;

    NSLog(@"write flash header");

    if (![self writeFlashHeader:data.length]) {
        NSLog(@"Failed to write flash header");
        return false;
    }

    [progress addProgress];

    NSLog(@"write flash data");

    if (![self writeFlash:data.bytes size:(int)data.length withProgress:progress]) {
        NSLog(@"Failed to write flash data");
        return false;
    }

    NSLog(@"finish flash upgrade");

    if (![self finishFlashUpgrade:(data.length + 8)]) {
        NSLog(@"Failed to finishUpgrade");
        return false;
    }

    NSLog(@"write flash disable");

    [self setFlashWriteEnable:false];

    [progress setProgressMax:100];
    [progress finish];

    return true;
}

- (BOOL)upgradeFirmware:(nonnull const char *)pathname
           withProgress:(nullable CavanProgressManager *)progress {
    if (mUpgradeBusy) {
        NSLog(@"upgrade busy");
        return false;
    }

    mUpgradeBusy = true;
    BOOL result = [self upgradeFirmwareSafe:pathname withProgress:progress];
    mUpgradeBusy = false;

    return result;
}

- (NSData *)readBdAddress {
    NSData *data = [mCommand readDataWithType:JWAOO_TOY_CMD_FLASH_READ_BD_ADDR];
    if (data == nil || data.length != 6) {
        return nil;
    }

    return data;
}

- (NSString *)readBdAddressString {
    NSData *data = [self readBdAddress];
    if (data == nil) {
        return nil;
    }

    const uint8_t *bytes = data.bytes;

    return [NSString stringWithFormat:@"%02x:%02x:%02x:%02x:%02x:%02x",
            bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]];
}

- (BOOL)writeBdAddressWithBytes:(const uint8_t *)bytes {
    if (![self setFlashWriteEnable:true]) {
        return false;
    }

    if (![mCommand readBoolWithType:JWAOO_TOY_CMD_FLASH_WRITE_BD_ADDR withBytes:bytes length:6]) {
        return false;
    }

    return [self setFlashWriteEnable:false];
}

- (BOOL)writeBdAddressWithString:(NSString *)addr {
    const char *text = [addr cStringUsingEncoding:NSASCIIStringEncoding];
    int values[6];

    if (sscanf(text, "%02x:%02x:%02x:%02x:%02x:%02x",
               values, values + 1, values + 2, values + 3, values + 4, values + 5) == 6) {
        uint8_t bytes[6];

        for (int i = 0; i < 6; i++) {
            bytes[i] = values[i];
        }

        return [self writeBdAddressWithBytes:bytes];
    }

    NSLog(@"Invalid format");

    return false;
}

- (BOOL)setKeyClickEnable:(BOOL)enable {
    return [mCommand readBoolWithType:JWAOO_TOY_CMD_KEY_CLICK_ENABLE withBool:enable];
}

- (BOOL)setKeyLongClickEnable:(BOOL)enable {
    return [mCommand readBoolWithType:JWAOO_TOY_CMD_KEY_LONG_CLICK_ENABLE withBool:enable];
}

- (BOOL)setKeyLongClickEnable:(BOOL)enable
                 withDelay:(uint16_t)delay {
    return [mCommand readBoolWithType:JWAOO_TOY_CMD_KEY_LONG_CLICK_ENABLE withBool:enable withDelay16:delay];
}

- (BOOL)setKeyMultiClickEnable:(BOOL)enable {
    return [mCommand readBoolWithType:JWAOO_TOY_CMD_KEY_MULTI_CLICK_ENABLE withBool:enable];
}

- (BOOL)setKeyMultiClickEnable:(BOOL)enable
                  withDelay:(uint16_t)delay {
    return [mCommand readBoolWithType:JWAOO_TOY_CMD_KEY_MULTI_CLICK_ENABLE withBool:enable withDelay16:delay];
}

- (BOOL)setKeyLock:(BOOL)enable {
    return [mCommand readBoolWithType:JWAOO_TOY_CMD_KEY_LOCK withBool:enable];
}

- (BOOL)setKeyReportEnable:(uint8_t)mask {
    return [mCommand readBoolWithType:JWAOO_TOY_CMD_KEY_REPORT_ENABLE withValue8:mask];
}

- (BOOL)setLedEnable:(uint8_t)index
              enable:(BOOL)enable {
    uint8_t command[] = { JWAOO_TOY_CMD_LED_ENABLE, index, enable };
    return [mCommand readBoolWithBytes:command length:sizeof(command)];
}

- (BOOL)setMotoEventEnable:(BOOL)enable {
    return [mCommand readBoolWithType:JWAOO_TOY_CMD_MOTO_EVENT_ENABLE withBool:enable];
}

- (JwaooToyMotoMode *)getMotoMode {
    NSData *response = [mCommand readDataWithType:JWAOO_TOY_CMD_MOTO_GET_MODE];
    if (response == nil || response.length != 2) {
        return nil;
    }

    const uint8_t *bytes = response.bytes;

    return [[JwaooToyMotoMode alloc] initWithMode:bytes[0] withSpeed:bytes[1]];
}

- (BOOL)setMotoMode:(uint8_t)mode
          withSpeed:(uint8_t)speed {
    uint8_t command[] = { JWAOO_TOY_CMD_MOTO_SET_MODE, mode, speed };

    return [mCommand readBoolWithBytes:command length:sizeof(command)];
}

@end
