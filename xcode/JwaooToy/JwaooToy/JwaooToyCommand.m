//
//  JwaooToyCommand.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/19.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "JwaooToyCommand.h"
#import "JwaooToyResponse.h"

@implementation JwaooToyCommand

- (JwaooToyCommand *)initWithBleChar:(CavanBleChar *)bleChar {
    if (self = [super init]) {
        mChar = bleChar;
    }

    return self;
}

- (JwaooToyResponse *)send:(NSData *)command {
    uint8_t type = ((uint8_t *) command.bytes)[0];

    for (int i = 0; i < 10; i++) {
        NSData *response = [mChar sendCommand:command];
        if (response == nil) {
            break;
        }

        if (response.length < 2) {
            NSLog(@"Invalid response length: %lu", (unsigned long)response.length);
            continue;
        }

        const uint8_t *bytes = response.bytes;
        if (bytes[0] == type) {
            return [[JwaooToyResponse alloc] initWithBytes:bytes length:response.length];
        }

        NSLog(@"Invalid response command type: %d, expect = %d", bytes[0], type);
    }

    return nil;
}

- (JwaooToyResponse *)sendWithBytes:(const void *)bytes
                 length:(NSUInteger)length {
    NSData *data = [[NSData alloc] initWithBytes:bytes length:length];

    return [self send:data];
}

- (JwaooToyResponse *)sendWithType:(uint8_t)type {
    NSData *data = [[NSData alloc] initWithBytes:&type length:1];

    return [self send:data];
}

- (JwaooToyResponse *)sendWithType:(uint8_t)type
               withBytes:(const void *)bytes
                 length:(NSUInteger)length {
    NSMutableData *data = [[NSMutableData alloc] initWithCapacity:(length + 1)];

    [data appendBytes:&type length:1];
    [data appendBytes:bytes length:length];

    return [self send:data];
}

- (JwaooToyResponse *)sendWithType:(uint8_t)type
                          withBool:(BOOL)value {
    return [self sendWithType:type withValue8:value];
}

- (JwaooToyResponse *)sendWithType:(uint8_t)type
                          withBool:(BOOL)enable
                         withDelay16:(uint16_t)delay {
    struct jwaoo_toy_command command;

    command.type = type;
    command.enable = enable;
    command.delay16 = delay;

    return [self sendWithBytes:&command length:4];
}

- (JwaooToyResponse *)sendWithType:(uint8_t)type
                          withBool:(BOOL)enable
                       withDelay32:(uint32_t)delay {
    struct jwaoo_toy_command command;

    command.type = type;
    command.enable = enable;
    command.delay32 = delay;

    return [self sendWithBytes:&command length:6];
}

- (JwaooToyResponse *)sendWithType:(uint8_t)type
                        withValue8:(uint8_t)value {
    struct jwaoo_toy_command command;

    command.type = type;
    command.value8 = value;

    return [self sendWithBytes:&command length:2];
}

- (JwaooToyResponse *)sendWithType:(uint8_t)type
                       withValue16:(uint16_t)value {
    struct jwaoo_toy_command command;

    command.type = type;
    command.value16 = value;

    return [self sendWithBytes:&command length:3];
}

- (JwaooToyResponse *)sendWithType:(uint8_t)type
                      withValue32:(uint32_t)value {
    struct jwaoo_toy_command command;

    command.type = type;
    command.value32 = value;

    return [self sendWithBytes:&command length:5];
}

- (JwaooToyResponse *)sendWithType:(uint8_t)type
                          withText:(NSString *)text {
    NSData *data = [text dataUsingEncoding:NSASCIIStringEncoding];
    return [self sendWithType:type withBytes:data.bytes length:data.length];
}

// ================================================================================

- (BOOL)readBoolWithBytes:(const void *)bytes
          length:(NSUInteger)length {
    JwaooToyResponse *response = [self sendWithBytes:bytes length:length];
    return [JwaooToyResponse getBool:response];
}

- (BOOL)readBoolWithType:(uint8_t)type {
    JwaooToyResponse *response = [self sendWithType:type];
    return [JwaooToyResponse getBool:response];
}

- (uint8_t)readValueWithType8:(uint8_t)type
                     defValue:(uint8_t)defValue {
    JwaooToyResponse *response = [self sendWithType:type];
    return [JwaooToyResponse getValue8:response defValue:defValue];
}

- (uint8_t)readValueWithType8:(uint8_t)type {
    return [self readValueWithType8:type defValue:0];
}

- (uint16_t)readValueWithType16:(uint8_t)type
                       defValue:(uint16_t)defValue {
    JwaooToyResponse *response = [self sendWithType:type];
    return [JwaooToyResponse getValue16:response defValue:defValue];
}

- (uint16_t)readValueWithType16:(uint8_t)type {
    return [self readValueWithType16:type defValue:0];
}

- (uint32_t)readValueWithType32:(uint8_t)type
                       defValue:(uint32_t)defValue {
    JwaooToyResponse *response = [self sendWithType:type];
    return [JwaooToyResponse getValue32:response defValue:defValue];
}

- (uint32_t)readValueWithType32:(uint8_t)type {
    return [self readValueWithType32:type defValue:0];
}

- (NSString *)readTextWithType:(uint8_t)type {
    JwaooToyResponse *response = [self sendWithType:type];
    return [JwaooToyResponse getText:response];
}

- (NSData *)readDataWithType:(uint8_t)type {
    JwaooToyResponse *response = [self sendWithType:type];
    return [JwaooToyResponse getData:response];
}

- (BOOL)readBoolWithType:(uint8_t)type
                   withBool:(BOOL)value {
    JwaooToyResponse *response = [self sendWithType:type withBool:value];
    return [JwaooToyResponse getBool:response];
}

- (BOOL)readBoolWithType:(uint8_t)type
                 withValue8:(uint8_t)value {
    JwaooToyResponse *response = [self sendWithType:type withValue8:value];
    return [JwaooToyResponse getBool:response];
}

- (BOOL)readBoolWithType:(uint8_t)type
                withValue16:(uint16_t)value {
    JwaooToyResponse *response = [self sendWithType:type withValue16:value];
    return [JwaooToyResponse getBool:response];
}

- (BOOL)readBoolWithType:(uint8_t)type
                withValue32:(uint32_t)value {
    JwaooToyResponse *response = [self sendWithType:type withValue32:value];
    return [JwaooToyResponse getBool:response];
}

- (BOOL)readBoolWithType:(uint8_t)type
               withBytes:(const void *)bytes
                   length:(NSUInteger)length {
    JwaooToyResponse *response = [self sendWithType:type withBytes:bytes length:length];
    return [JwaooToyResponse getBool:response];
}

- (BOOL)readBoolWithType:(uint8_t)type
                withText:(NSString *)text {
    JwaooToyResponse *response = [self sendWithType:type withText:text];
    return [JwaooToyResponse getBool:response];
}

- (BOOL)readBoolWithType:(uint8_t)type
                withBool:(BOOL)enable
             withDelay16:(uint16_t)delay {
    JwaooToyResponse *response = [self sendWithType:type withBool:enable withDelay16:delay];
    return [JwaooToyResponse getBool:response];
}

- (BOOL)readBoolWithType:(uint8_t)type
                withBool:(BOOL)enable
             withDelay32:(uint32_t)delay {
    JwaooToyResponse *response = [self sendWithType:type withBool:enable withDelay32:delay];
    return [JwaooToyResponse getBool:response];
}

@end
