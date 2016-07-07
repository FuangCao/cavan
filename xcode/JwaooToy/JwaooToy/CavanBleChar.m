//
//  CavanBleChar.m
//  TestBle
//
//  Created by 曹福昂 on 16/7/7.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "CavanBleChar.h"

@implementation CavanBleChar

- (CavanBleChar *)initWithCharacteristic:(CBCharacteristic *)characteristic
                                  peripheral:(CBPeripheral *)peripheral; {
    if (self = [super init]) {
        mReadCond = [NSCondition new];
        mWriteCond = [NSCondition new];
        mPeripheral = peripheral;
        mChar = characteristic;
    }

    return self;
}

- (void)postNotification {
}

- (void)setWriteStatus:(NSError *)error {
    if (error == nil) {
        mWriteError = 0;
    } else {
        mWriteError = error.code;
    }
    
    // NSLog(@"mWriteError = %ld", (long)mWriteError);

    [mWriteCond signal];
}

- (void)setReadStatus:(NSError *)error {
    if (error == nil) {
        mReadError = 0;
    } else {
        mReadError = error.code;
    }
    
    // NSLog(@"mReadError = %ld", (long)mReadError);

    [mReadCond signal];
}

- (NSData *)readData {
    NSData *value;

    @synchronized (self) {
        [mReadCond lock];
        [mPeripheral readValueForCharacteristic:mChar];

        if ([mReadCond waitUntilDate:[[NSDate alloc] initWithTimeIntervalSinceNow:2.0]] && mReadError == 0) {
            value = mChar.value;
        } else {
            value = nil;
        }

        [mReadCond unlock];
    }

    return value;
}

- (BOOL)writeFrame:(NSData *)data {
    BOOL success = FALSE;

    @synchronized (self) {
        [mWriteCond lock];

        for (int i = 0; i < 5; i++) {
            [mPeripheral writeValue:data forCharacteristic:mChar type:CBCharacteristicWriteWithResponse];
            if ([mWriteCond waitUntilDate:[[NSDate alloc] initWithTimeIntervalSinceNow:1.0]]) {
                success = (mWriteError == 0);
                break;
            }

            NSLog(@"writeFrame timeout%d", i);
        }

        [mWriteCond unlock];
    }

    return success;
}

- (BOOL)writeData:(const void *)bytes
           length:(NSUInteger)length {
    @synchronized (self) {
        while (length > CAVAN_BLE_FRAME_SIZE) {
            NSData *data = [[NSData alloc] initWithBytes:bytes length:length];
            
            if (![self writeFrame:data]) {
                return FALSE;
            }

            bytes += CAVAN_BLE_FRAME_SIZE;
            length -= CAVAN_BLE_FRAME_SIZE;
        }

        if (length > 0) {
            NSData *data = [[NSData alloc] initWithBytes:bytes length:length];
            
            return [self writeFrame:data];
        }
    }

    return TRUE;
}

- (NSData *)sendCommand:(NSData *)command {
    @synchronized (self) {
        if ([self writeFrame:command]) {
            return [self readData];
        }
    }

    return nil;
}

@end
