//
//  JwaooToyResponse.m
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/19.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "JwaooToyResponse.h"

@implementation JwaooToyResponse

@synthesize length = mLength;
@synthesize response = mResponse;

- (JwaooToyResponse *)initWithBytes:(const void *)bytes
                             length:(NSUInteger)length {
    if (self = [super init]) {
        mLength = length;
        mResponse = bytes;
    }

    return self;
}

- (nonnull JwaooToyResponse *)initWithData:(nonnull NSData *)data {
    return [self initWithBytes:data.bytes length:data.length];
}

- (uint8_t)command {
    return mResponse->command;
}

- (uint8_t)type {
    return mResponse->type;
}

// ================================================================================

- (BOOL)getBool {
    if (mLength != 3 || mResponse->type != JWAOO_TOY_RSP_BOOL) {
        return false;
    }

    return mResponse->value8 != 0;;
}

- (uint8_t)getValue8:(uint8_t)defValue {
    if (mLength != 3 || mResponse->type != JWAOO_TOY_RSP_U8) {
        return defValue;
    }

    return mResponse->value8;
}

- (uint16_t)getValue16:(uint16_t)defValue {
    if (mLength != 4 || mResponse->type != JWAOO_TOY_RSP_U16) {
        return defValue;
    }

    return mResponse->value16;
}

- (uint32_t)getValue32:(uint32_t)defValue {
    if (mLength != 6 || mResponse->type != JWAOO_TOY_RSP_U32) {
        return defValue;
    }

    return mResponse->value32;
}

- (NSString *)getText {
    if (mResponse->type != JWAOO_TOY_RSP_TEXT) {
        return nil;
    }

    return [[NSString alloc] initWithBytes:mResponse->text length:(mLength - 2) encoding:NSASCIIStringEncoding];
}

- (NSData *)getData {
    if (mResponse->type != JWAOO_TOY_RSP_DATA) {
        return nil;
    }

    return [[NSData alloc] initWithBytes:mResponse->bytes length:(mLength - 2)];
}

// ================================================================================

+ (BOOL)getBool:(JwaooToyResponse *)response {
    return response != nil && [response getBool];
}

+ (uint8_t)getValue8:(JwaooToyResponse *)response
            defValue:(uint8_t)defValue {
    if (response == nil) {
        return defValue;
    }

    return [response getValue8:defValue];
}

+ (uint16_t)getValue16:(JwaooToyResponse *)response
              defValue:(uint16_t)defValue {
    if (response == nil) {
        return defValue;
    }

    return [response getValue16:defValue];
}

+ (uint32_t)getValue32:(JwaooToyResponse *)response
              defValue:(uint32_t)defValue {
    if (response == nil) {
        return defValue;
    }

    return [response getValue32:defValue];
}

+ (NSString *)getText:(JwaooToyResponse *)response {
    if (response == nil) {
        return nil;
    }

    return [response getText];
}

+ (NSData *)getData:(JwaooToyResponse *)response {
    if (response == nil) {
        return nil;
    }

    return [response getData];
}

@end
