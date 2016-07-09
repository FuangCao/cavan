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

@interface CavanBleGatt : CBCentralManager <CBCentralManagerDelegate, CBPeripheralDelegate> {
    CBUUID *mUUID;
    CBPeripheral *mPeripheral;

    NSString *mName;
    NSMutableDictionary *mDictChars;
}

- (nullable CavanBleGatt *)initWithName:(nullable NSString *)name
                          uuid:(nullable CBUUID *)uuid;

- (void)startScan;
- (void)addBleChar:(nonnull CavanBleChar *)bleChar
          withUUID:(nonnull CBUUID *)uuid;
- (nullable CavanBleChar *)createBleChar:(nonnull CBCharacteristic *)characteristic
                       degelate:(nullable id<CavanBleCharDelegate>)delegate;
- (void)disconnect;

@end
