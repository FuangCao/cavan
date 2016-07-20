//
//  JwaooToyResponse.h
//  JwaooToy
//
//  Created by 曹福昂 on 16/7/19.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "JwaooToyCommand.h"

@interface JwaooToyResponse : NSObject {
    NSUInteger mLength;
    const struct jwaoo_toy_response *mResponse;
}

@property NSUInteger length;
@property (nonnull) const struct jwaoo_toy_response *response;
@property (readonly) uint8_t command;
@property (readonly) uint8_t type;

- (nonnull JwaooToyResponse *)initWithBytes:(nonnull const void *)bytes
                             length:(NSUInteger)length;
- (nonnull JwaooToyResponse *)initWithData:(nonnull NSData *)data;
- (BOOL)getBool;
- (uint8_t)getValue8:(uint8_t)defValue;
- (uint16_t)getValue16:(uint16_t)defValue;
- (uint32_t)getValue32:(uint32_t)defValue;
- (nullable NSString *)getText;
- (nullable NSData *)getData;

// ================================================================================

+ (BOOL)getBool:(nullable JwaooToyResponse *)response;
+ (uint8_t)getValue8:(nullable JwaooToyResponse *)response
            defValue:(uint8_t)defValue;
+ (uint16_t)getValue16:(nullable JwaooToyResponse *)response
              defValue:(uint16_t)defValue;
+ (uint32_t)getValue32:(nullable JwaooToyResponse *)response
              defValue:(uint32_t)defValue;
+ (nullable NSString *)getText:(nullable JwaooToyResponse *)response;
+ (nullable NSData *)getData:(nullable JwaooToyResponse *)response;

@end
