//
//  JwaooToy.m
//  TestBle
//
//  Created by 曹福昂 on 16/7/7.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "JwaooBleToy.h"
#import "CavanHexFile.h"
#import "Mpu6050Sensor.h"

@implementation JwaooBleToy

- (int) freq {
    return mParser.freq;
}

- (int) depth {
    return mParser.depth;
}

- (JwaooBleToy *)initWithDelegate:(id<JwaooBleToyDelegate>)delegate {
    if (self = [super initWithName:@"JwaooToy" uuid:JWAOO_TOY_UUID_SERVICE]) {
        mDelegate = delegate;
        mSensor = [Mpu6050Sensor new];

        mParser = [[JwaooToyParser alloc] initWithValueFuzz:JWAOO_TOY_VALUE_FUZZ withTimeFuzz:JWAOO_TOY_TIME_FUZZ];
        [mParser setDepthSelector:@selector(onDepthChanged:) withTarget:self];
        [mParser setFreqSelector:@selector(onFreqChanged:) withTarget:self];
    }

    return self;
}

- (void)onFreqChanged:(NSNumber *)freq {
    if ([mDelegate respondsToSelector:@selector(didFreqChanged:)]) {
        [mDelegate didFreqChanged:freq.intValue];
    }
}

- (void)onDepthChanged:(NSNumber *)depth {
    if ([mDelegate respondsToSelector:@selector(didDepthChanged:)]) {
        [mDelegate didDepthChanged:depth.intValue];
    }
}

- (void)onEventReceived:(CavanBleChar *)bleChar {
    if ([mDelegate respondsToSelector:@selector(didEventReceived:)]) {
        [mDelegate didEventReceived:bleChar];
    }
}

- (void)onSensorDataReceived:(CavanBleChar *)bleChar {
    [mSensor putBytes:bleChar.bytes];
    [mParser putSensorData:mSensor];

    if ([mDelegate respondsToSelector:@selector(didSensorDataReceived:)]) {
        [mDelegate didSensorDataReceived:bleChar];
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

    mCharCommand = mCharEvent = mCharFlash = mCharSensor = nil;

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

    NSString *identify = [self doIdentify];
    if (identify == nil) {
        NSLog(@"Failed to doIdentify");
        return false;
    }

    NSLog(@"identify = %@", identify);

    if (![identify isEqualToString:JWAOO_TOY_IDENTIFY]) {
        NSLog(@"Invalid identify");
        return false;
    }

    return [mDelegate doInitialize:self];
}

// ================================================================================

+ (BOOL)parseResponseBool:(NSData *)response {
    if (response == nil || response.length != 2) {
        return FALSE;
    }

    struct jwaoo_toy_command *command = (struct jwaoo_toy_command *) response.bytes;

    return command->type == JWAOO_TOY_RSP_BOOL && command->value8 > 0;
}

+ (uint8_t)parseResponseValue8:(NSData *)response {
    if (response == nil || response.length != 2) {
        return 0;
    }

    struct jwaoo_toy_command *command = (struct jwaoo_toy_command *) response.bytes;
    if (command->type == JWAOO_TOY_RSP_U8) {
        return command->value8;
    }

    NSLog(@"Invalid response type = %d", command->type);

    return 0;
}

+ (uint16_t)parseResponseValue16:(NSData *)response {
    if (response == nil || response.length != 3) {
        return 0;
    }

    struct jwaoo_toy_command *command = (struct jwaoo_toy_command *) response.bytes;
    if (command->type == JWAOO_TOY_RSP_U16) {
        return command->value16;
    }

    NSLog(@"Invalid response type = %d", command->type);

    return 0;
}

+ (uint32_t)parseResponseValue32:(NSData *)response {
    if (response == nil || response.length != 5) {
        return 0;
    }

    struct jwaoo_toy_command *command = (struct jwaoo_toy_command *) response.bytes;
    if (command->type == JWAOO_TOY_RSP_U32) {
        return command->value32;
    }

    NSLog(@"Invalid response type = %d", command->type);

    return 0;
}

+ (NSString *)parseResponseText:(NSData *)response {
    if (response == nil || response.length < 2) {
        return nil;
    }

    struct jwaoo_toy_command *command = (struct jwaoo_toy_command *) response.bytes;
    if (command->type == JWAOO_TOY_RSP_TEXT) {
        return [[NSString alloc] initWithBytes:command->text length:response.length - 1 encoding:NSASCIIStringEncoding];
    }

    NSLog(@"Invalid response type = %d", command->type);

    return nil;
}

// ================================================================================

- (NSData *)sendCommand:(NSData *)command {
    if (mCharCommand == nil) {
        return nil;
    }

    return [mCharCommand sendCommand:command];
}

- (NSData *)sendCommand:(const void *)command
                 length:(NSUInteger)length {
    NSData *data = [[NSData alloc] initWithBytes:command length:length];

    return [self sendCommand:data];
}

- (BOOL)sendCommandReadBool:(const void *)command
                     length:(NSUInteger)length {
    NSData *response = [self sendCommand:command length:length];
    return [self.class parseResponseBool:response];
}

- (NSData *)sendEmptyCommand:(uint8_t) command {
    NSData *data = [[NSData alloc] initWithBytes:&command length:1];

    return [self sendCommand:data];
}

- (NSData *)sendCommand:(uint8_t)type
               withBool:(BOOL)value {
    return [self sendCommand:type withValue8:value];
}

- (NSData *)sendCommand:(uint8_t)type
             withValue8:(uint8_t)value {
    struct jwaoo_toy_command command = { type, value };

    return [self sendCommand:&command length:2];
}

- (NSData *)sendCommand:(uint8_t)type
            withValue16:(uint16_t)value {
    struct jwaoo_toy_command command = { type, value };

    return [self sendCommand:&command length:3];
}

- (NSData *)sendCommand:(uint8_t)type
            withValue32:(uint32_t)value {
    struct jwaoo_toy_command command = { type, value };

    return [self sendCommand:&command length:5];
}

- (BOOL)sendEmptyCommandReadBool:(uint8_t)type {
    NSData *response = [self sendEmptyCommand:type];
    return [self.class parseResponseBool:response];
}

- (uint8_t)sendEmptyCommandReadValue8:(uint8_t)type {
    NSData *response = [self sendEmptyCommand:type];
    return [self.class parseResponseValue8:response];
}

- (uint16_t)sendEmptyCommandReadValue16:(uint8_t)type {
    NSData *response = [self sendEmptyCommand:type];
    return [self.class parseResponseValue16:response];
}

- (uint32_t)sendEmptyCommandReadValue32:(uint8_t)type {
    NSData *response = [self sendEmptyCommand:type];
    return [self.class parseResponseValue32:response];
}

- (NSString *)sendEmptyCommandReadText:(uint8_t)type {
    NSData *response = [self sendEmptyCommand:type];
    return [self.class parseResponseText:response];
}

- (BOOL)sendEmptyCommandReadBool:(uint8_t)type
                        withBool:(BOOL)value {
    NSData *response = [self sendCommand:type withBool:value];
    return [self.class parseResponseBool:response];
}

- (BOOL)sendEmptyCommandReadBool:(uint8_t)type
                      withValue8:(uint8_t)value {
    NSData *response = [self sendCommand:type withValue8:value];
    return [self.class parseResponseBool:response];
}

- (BOOL)sendEmptyCommandReadBool:(uint8_t)type
                     withValue16:(uint16_t)value {
    NSData *response = [self sendCommand:type withValue16:value];
    return [self.class parseResponseBool:response];
}

- (BOOL)sendEmptyCommandReadBool:(uint8_t)type
                     withValue32:(uint32_t)value {
    NSData *response = [self sendCommand:type withValue32:value];
    return [self.class parseResponseBool:response];
}

// ================================================================================

- (NSString *)doIdentify {
    return [self sendEmptyCommandReadText:JWAOO_TOY_CMD_IDENTIFY];
}

- (NSString *)readBuildDate {
    return [self sendEmptyCommandReadText:JWAOO_TOY_CMD_BUILD_DATE];
}

- (uint32_t)readVersion {
    return [self sendEmptyCommandReadValue32:JWAOO_TOY_CMD_VERSION];
}

- (BOOL)doReboot {
    return [self sendEmptyCommandReadBool:JWAOO_TOY_CMD_REBOOT];
}

- (BOOL)setSensorEnable:(BOOL)enable {
    return [self sendEmptyCommandReadBool:JWAOO_TOY_CMD_SENSOR_ENABLE withBool:enable];
}

- (BOOL)setSensorDelay:(uint32_t)delay {
    return [self sendEmptyCommandReadBool:JWAOO_TOY_CMD_SENSOR_SET_DELAY withValue32:delay];
}

- (uint32_t)getFlashId {
    return [self sendEmptyCommandReadValue32:JWAOO_TOY_CMD_FLASH_ID];
}

- (uint32_t)getFlashSize {
    return [self sendEmptyCommandReadValue32:JWAOO_TOY_CMD_FLASH_SIZE];
}

- (uint32_t)getFlashPageSize {
    return [self sendEmptyCommandReadValue32:JWAOO_TOY_CMD_FLASH_PAGE_SIZE];
}

- (BOOL)setFlashWriteEnable:(BOOL)enable {
    return [self sendEmptyCommandReadBool:JWAOO_TOY_CMD_FLASH_WRITE_ENABLE withBool:enable];
}

- (BOOL)eraseFlash {
    return [self sendEmptyCommandReadBool:JWAOO_TOY_CMD_FLASH_ERASE];
}

- (BOOL)startFlashUpgrade {
    return [self sendEmptyCommandReadBool:JWAOO_TOY_CMD_FLASH_WRITE_START];
}

- (BOOL)finishFlashUpgrade:(uint16_t)length {
    uint8_t command[] = { JWAOO_TOY_CMD_FLASH_WRITE_FINISH, mFlashCrc, length & 0xFF, length >> 8 };
    return [self sendCommandReadBool:command length:sizeof(command)];
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

    NSLog(@"readBinData");

    NSData *data = [file readBinData];
    if (data == nil) {
        NSLog(@"Failed to readBinData");
        return FALSE;
    }

    [progress addProgress];

    NSLog(@"length = %ld = 0x%08lx", data.length, data.length);
    NSLog(@"setFlashWriteEnable true");

    if (![self setFlashWriteEnable:true]) {
        NSLog(@"Failed to setWriteEnable true");
        return false;
    }

    [progress addProgress];

    NSLog(@"startFlashUpgrade");

    if (![self startFlashUpgrade]) {
        NSLog(@"Failed to startUpgrade");
        return false;
    }

    [progress addProgress];

    NSLog(@"doFlashErase");

    if (![self eraseFlash]) {
        NSLog(@"Failed to doErase");
        return false;
    }

    [progress addProgress];

    mFlashCrc = 0xFF;

    NSLog(@"writeFlashHeader");

    if (![self writeFlashHeader:data.length]) {
        NSLog(@"Failed to write flash header");
        return false;
    }

    [progress addProgress];

    NSLog(@"writeFlash data");

    if (![self writeFlash:data.bytes size:(int)data.length withProgress:progress]) {
        NSLog(@"Failed to write flash data");
        return false;
    }

    NSLog(@"finishFlashUpgrade");

    if (![self finishFlashUpgrade:(data.length + 8)]) {
        NSLog(@"Failed to finishUpgrade");
        return false;
    }

    NSLog(@"setFlashWriteEnable false");

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

@end
