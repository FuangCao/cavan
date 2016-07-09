//
//  CavanHexFile.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/8.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "CavanHexFile.h"

@implementation CavanHexFile

- (CavanHexFile *)initWithPath:(const char *)pathname
                          mode:(const char *)mode {
    if (self = [super init]) {
        if (mode == nil) {
            mode = "r";
        }

        fp = fopen(pathname, mode);
        if (fp == nil) {
            NSLog(@"Failed to fopen: %s", pathname);
            return nil;
        }
    }

    return self;
}

- (void)dealloc {
    if (fp != nil) {
        fclose(fp);
    }
}

+ (uint8_t)parseChar:(char)c {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }

    c |= 0x20;

    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 10;
    }

    return 0;
}

+ (uint8_t)parseHexByte:(const char *)text {
    return [self parseChar:text[0]] << 4 | [self parseChar:text[1]];
}

+ (int)parseHexText:(const char *)text
            data:(uint8_t *)data
            dateSize:(int)dataSize {
    uint8_t *dataBak;
    uint8_t *dataEnd = data + dataSize;

    for (dataBak = data; text[0] && text[1] && data < dataEnd; text += 2, data++) {
        *data = [self parseHexByte:text];
    }

    return (int)(data - dataBak);
}

+ (uint32_t)getHexValue:(const uint8_t *)data
                size:(int)size {
    uint32_t value;
    const uint8_t *dataEnd = data + size;

    for (value = 0; data < dataEnd; data++) {
        value = value << 8 | *data;
    }

    return value;
}

+ (uint16_t)endianConvert16:(uint16_t)value {
    return value << 8 | value >> 8;
}

+ (uint32_t)endianConvert32:(uint32_t)value {
    return ((uint32_t)[self endianConvert16:value & 0xFFFF]) << 16 | [self endianConvert16:value >> 16];
}

- (NSData *)readBinData {
    char text[64];
    uint32_t offset;
    uint8_t lineData[32];
    NSMutableData *binData = [NSMutableData new];
    struct cavan_hex_line *line = (struct cavan_hex_line *)lineData;

    while (fgets(text, sizeof(text), fp)) {
        if (text[0] != ':') {
            NSLog(@"Invalid char: %c", text[0]);
            return nil;
        }

        int length = [self.class parseHexText:text + 1 data:lineData dateSize:sizeof(lineData)];
        if (length < line->length + 5) {
            NSLog(@"Invalid line size: %d", length);
            return nil;
        }

        switch (line->type) {
            case 0x00:
                offset = [self.class getHexValue:line->offset size:sizeof(line->offset)];
                if (offset > binData.length) {
                    NSLog(@"seek to: %d", offset);
                    [binData setLength:offset];
                }

                [binData appendBytes:line->data length:line->length];
                break;

            case 0x01:
                return binData;

            case 0x02:
                break;

            case 0x03:
                break;

            case 0x04:
                NSLog(@"offset = 0x%04x", [self.class getHexValue:line->data size:line->length]);
                break;

            case 0x05:
                NSLog(@"base address = 0x%08x", [self.class getHexValue:line->data size:line->length]);
                break;

            default:
                NSLog(@"Invalid type = %d", line->type);
                return nil;
        }
    }

    return nil;
}

@end
