//
//  CavanBleChar.h
//  TestBle
//
//  Created by 曹福昂 on 16/7/7.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import "CavanProgressManager.h"

#define CAVAN_BLE_FRAME_SIZE    20

@class CavanBleGatt;

@interface CavanBleChar : NSObject {
    CBCharacteristic *mChar;
    CavanBleGatt *mGatt;
    CBPeripheral *mPeripheral;

    NSError *mWriteError;
    NSCondition *mWriteCond;
    NSError *mReadError;
    NSCondition *mReadCond;

    SEL mNotifySelector;
    NSObject *mNotifyTarget;
}

@property (nonnull, readonly, retain) NSData *data;
@property (nonnull, readonly) const void *bytes;

- (nullable CavanBleChar *)initWithCharacteristic:(nonnull CBCharacteristic *)characteristic
                                       gatt:(nonnull CavanBleGatt *)gatt;
- (BOOL)canRead;
- (BOOL)canWriteWithoutResponse;
- (BOOL)canWrite;
- (BOOL)canNotify;
- (BOOL)canIndicate;
- (void)enableNotifyWithSelector:(nonnull SEL)selector
               withTarget:(nullable NSObject *)target;
- (void)setWriteStatus:(nullable NSError *)error;
- (void)setReadStatus:(nullable NSError *)error;
- (nullable NSData *)readData;
- (BOOL)writeDataNoRsp:(nonnull NSData *)data;
- (BOOL)writeFrame:(nonnull NSData *)data;
- (BOOL)writeData:(nonnull const void *)bytes
           length:(NSUInteger)length
     withProgress:(nullable CavanProgressManager *)progress;
- (nullable NSData *)sendCommand:(nonnull NSData *)command;

@end
