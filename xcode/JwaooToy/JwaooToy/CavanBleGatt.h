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
#define CAVAN_BLE_CONN_TIME     2.0

@interface CavanBleGatt : CBCentralManager <CBCentralManagerDelegate, CBPeripheralDelegate> {
    CBUUID *mUUID;
    NSArray *mNames;
    CBService *mService;
    CBPeripheral *mPeripheral;
    dispatch_queue_t mQueue;

    NSNumber *mRssi;
    NSMutableDictionary *mDictChars;

    BOOL mConnected;
    BOOL mConnPending;
    BOOL mConnRunning;

    BOOL mInitPending;
    BOOL mInitRunning;
}

@property (nullable) NSNumber *rssi;
@property (nullable) CBService *service;
@property (nullable) CBPeripheral *peripheral;

- (nullable CavanBleGatt *)initWithNames:(nullable NSArray *)names
                          uuid:(nullable CBUUID *)uuids;

- (void)startScan;
- (void)addBleChar:(nonnull CavanBleChar *)bleChar
          withUUID:(nonnull CBUUID *)uuid;
- (nonnull CavanBleChar *)createBleChar:(nonnull CBCharacteristic *)characteristic;
- (void)disconnect;
- (BOOL)doInitialize;
- (void)onConnectStateChanged:(BOOL)connected;

@end
