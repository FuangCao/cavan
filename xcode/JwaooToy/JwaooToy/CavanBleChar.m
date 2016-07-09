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
                                  peripheral:(CBPeripheral *)peripheral
                                delegate:(id<CavanBleCharDelegate>)delegate {
    if (self = [super init]) {
        mReadCond = [NSCondition new];
        mWriteCond = [NSCondition new];
        mPeripheral = peripheral;
        mChar = characteristic;
        mDelegate = delegate;

        NSLog(@"uuid = %@, properties = 0x%08lx", mChar.UUID, (long)mChar.properties);

        if (mChar.properties & CBCharacteristicPropertyNotify) {
            [mPeripheral setNotifyValue:YES forCharacteristic:mChar];
        }
    }

    return self;
}

- (nullable NSData *)getData {
    return mChar.value;
}

- (nonnull CBCharacteristic *)getCharacteristic {
    return mChar;
}

- (nonnull CBPeripheral *)getPeripheral {
    return mPeripheral;
}

- (void)setWriteStatus:(NSError *)error {
    mWriteError = error;
    [mWriteCond signal];
}

- (void)setReadStatus:(NSError *)error {
    mReadError = error;

    if (error == nil && mChar.isNotifying) {
        [mDelegate didNotifyForCharacteristic:self];
    }

    [mReadCond signal];
}

- (NSData *)readData {
    NSData *value;

    @synchronized (self) {
        [mReadCond lock];
        [mPeripheral readValueForCharacteristic:mChar];

        if ([mReadCond waitUntilDate:[NSDate dateWithTimeIntervalSinceNow:2.0]] && mReadError == nil) {
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
            if ([mWriteCond waitUntilDate:[NSDate dateWithTimeIntervalSinceNow:1.0]]) {
                success = (mWriteError == nil);
                break;
            }

            NSLog(@"writeFrame timeout%d", i);
        }

        [mWriteCond unlock];
    }

    return success;
}

- (BOOL)writeData:(const void *)bytes
           length:(NSUInteger)length
     withProgress:(CavanProgressManager *)progress {

    [progress setValueRange:length];

    @synchronized (self) {
        while (1) {
            NSUInteger wrlen;

            if (length < CAVAN_BLE_FRAME_SIZE) {
                if (length == 0) {
                    break;
                }

                wrlen = length;
            } else {
                wrlen = CAVAN_BLE_FRAME_SIZE;
            }

            if (![self writeFrame:[[NSData alloc] initWithBytes:bytes length:wrlen]]) {
                NSLog(@"Failed to writeFrame");
                return FALSE;
            }

            bytes += wrlen;
            length -= wrlen;
            [progress addValue:wrlen];
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
