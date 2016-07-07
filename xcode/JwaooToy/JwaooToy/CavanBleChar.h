//
//  CavanBleChar.h
//  TestBle
//
//  Created by 曹福昂 on 16/7/7.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

#define CAVAN_BLE_FRAME_SIZE    20

@interface CavanBleChar : NSObject {
    CBCharacteristic *mChar;
    CBPeripheral *mPeripheral;


    NSInteger mWriteError;
    NSCondition *mWriteCond;
    NSInteger mReadError;
    NSCondition *mReadCond;
}

+ (NSInteger)decodeError:(nullable NSError *)error;

- (nullable CavanBleChar *)initWithCharacteristic:(nonnull CBCharacteristic *)characteristic
                                  peripheral:(nonnull CBPeripheral *)peripheral;
- (void)postNotification;
- (void)setWriteStatus:(nullable NSError *)error;
- (void)setReadStatus:(nullable NSError *)error;
- (nullable NSData *)readData;
- (BOOL)writeFrame:(nonnull NSData *)data;
- (BOOL)writeData:(nonnull const void *)bytes
           length:(NSUInteger)length;
- (nullable NSData *)sendCommand:(nonnull NSData *)command;

@end
