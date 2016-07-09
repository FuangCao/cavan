//
//  CavanHexFile.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/8.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>

#pragma pack(1)

struct cavan_hex_line {
    uint8_t length;
    uint8_t offset[2];
    uint8_t type;

    union {
        uint8_t data[0];
        uint8_t value8;
        uint16_t value16;
        uint32_t value32;
    };
};

#pragma pack()

@interface CavanHexFile : NSObject {
    FILE *fp;
}

- (nullable CavanHexFile *)initWithPath:(nonnull const char *)pathname
                          mode:(nullable const char *)mode;

+ (uint8_t)parseChar:(char)c;
+ (uint8_t)parseHexByte:(nonnull const char *)text;
+ (int)parseHexText:(nonnull const char *)text
               data:(nonnull uint8_t *)data
           dateSize:(int)dataSize;
+ (uint32_t)getHexValue:(nonnull const uint8_t *)data
                   size:(int)size;
+ (uint16_t)endianConvert16:(uint16_t)value;
+ (uint32_t)endianConvert32:(uint32_t)value;
- (nullable NSData *)readBinData;

@end
