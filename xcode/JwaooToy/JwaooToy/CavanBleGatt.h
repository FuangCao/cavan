//
//  CavanBleGatt.h
//  TestBle
//
//  Created by 曹福昂 on 16/7/7.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import "CavanBleChar.h"

#define CAVAN_BLE_SCAN_TIME     3.0

@interface CavanBleGatt : CBCentralManager <CBCentralManagerDelegate, CBPeripheralDelegate> {
    CBUUID *mUUID;
    NSDate *mDate;
    NSNumber *mRssi;
    CBPeripheral *mPeripheral;

    NSString *mName;
    NSMutableDictionary *mDictChars;
}

@property (nullable) NSNumber *rssi;
@property (nullable) CBPeripheral *peripheral;

- (nullable CavanBleGatt *)initWithName:(nullable NSString *)name
                          uuid:(nullable CBUUID *)uuid;

- (void)startScan;
- (void)addBleChar:(nonnull CavanBleChar *)bleChar
          withUUID:(nonnull CBUUID *)uuid;
- (nullable CavanBleChar *)createBleChar:(nonnull CBCharacteristic *)characteristic
                       degelate:(nullable id<CavanBleCharDelegate>)delegate;
- (void)disconnect;

@end
