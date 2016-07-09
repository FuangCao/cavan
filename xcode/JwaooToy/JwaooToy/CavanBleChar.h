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

@class CavanBleChar;

@protocol CavanBleCharDelegate <NSObject>
@required
- (void)didNotifyForCharacteristic:(nonnull CavanBleChar *)characteristic;
@end

@interface CavanBleChar : NSObject {
    CBCharacteristic *mChar;
    CBPeripheral *mPeripheral;
    id<CavanBleCharDelegate> mDelegate;

    NSError *mWriteError;
    NSCondition *mWriteCond;
    NSError *mReadError;
    NSCondition *mReadCond;
}

- (nullable CavanBleChar *)initWithCharacteristic:(nonnull CBCharacteristic *)characteristic
                                  peripheral:(nonnull CBPeripheral *)peripheral
                                         delegate:(nullable id<CavanBleCharDelegate>)delegate;
- (nullable NSData *)getData;
- (nonnull CBCharacteristic *)getCharacteristic;
- (nonnull CBPeripheral *)getPeripheral;

- (void)setWriteStatus:(nullable NSError *)error;
- (void)setReadStatus:(nullable NSError *)error;
- (nullable NSData *)readData;
- (BOOL)writeFrame:(nonnull NSData *)data;
- (BOOL)writeData:(nonnull const void *)bytes
           length:(NSUInteger)length
     withProgress:(nullable CavanProgressManager *)progress;
- (nullable NSData *)sendCommand:(nonnull NSData *)command;

@end