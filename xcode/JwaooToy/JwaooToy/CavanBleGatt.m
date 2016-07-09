//
//  CavanBleGatt.m
//  TestBle
//
//  Created by 曹福昂 on 16/7/7.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "CavanBleGatt.h"

@implementation CavanBleGatt

// ================================================================================

- (CavanBleGatt *)initWithName:(NSString *)name
                          uuid:(CBUUID *)uuid {
    if (self = [super initWithDelegate:self queue:dispatch_queue_create("com.cavan.bluetooth.gatt", DISPATCH_QUEUE_SERIAL)]) {
        mName = name;
        mUUID = uuid;
        mDictChars = [NSMutableDictionary new];
    }

    return self;
}

- (void)startScan {
    if (mUUID == nil) {
        [self scanForPeripheralsWithServices:nil options:nil];
    } else {
        [self scanForPeripheralsWithServices:@[ mUUID ] options:nil];
    }
}

- (void)addBleChar:(CavanBleChar *)bleChar
          withUUID:(CBUUID *)uuid {
    [mDictChars setObject:bleChar forKey:uuid];
}

- (CavanBleChar *)createBleChar:(CBCharacteristic *)characteristic
                       degelate:(id<CavanBleCharDelegate>)delegate {
    CavanBleChar *bleChar = [[CavanBleChar alloc] initWithCharacteristic:characteristic peripheral:mPeripheral delegate:delegate];

    [self addBleChar:bleChar withUUID:characteristic.UUID];

    return bleChar;
}

// ================================================================================

- (void)centralManagerDidUpdateState:(CBCentralManager *)central {
    NSLog(@"centralManagerDidUpdateState: state = %ld", (long)central.state);

    if (central.state == CBCentralManagerStatePoweredOn || central.state == CBCentralManagerStateResetting) {
        [self startScan];
    }
}

- (void)centralManager:(CBCentralManager *)central willRestoreState:(NSDictionary<NSString *, id> *)dict {
    NSLog(@"willRestoreState");
}

- (void)centralManager:(CBCentralManager *)central didRetrievePeripherals:(NSArray<CBPeripheral *> *)peripherals {
    NSLog(@"didRetrievePeripherals");
}

- (void)centralManager:(CBCentralManager *)central didRetrieveConnectedPeripherals:(NSArray<CBPeripheral *> *)peripherals {
    NSLog(@"didRetrieveConnectedPeripherals");
}

- (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral advertisementData:(NSDictionary<NSString *, id> *)advertisementData RSSI:(NSNumber *)RSSI {
    NSLog(@"didDiscoverPeripheral: %@", peripheral);
    if (mName == nil || [peripheral.name isEqualToString:mName]) {
        [self stopScan];
        mPeripheral = peripheral;
        [self connectPeripheral:peripheral options:nil];
    }
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral {
    NSLog(@"didConnectPeripheral: %@", peripheral);
    peripheral.delegate = self;
    [peripheral discoverServices:nil];
}

- (void)centralManager:(CBCentralManager *)central didFailToConnectPeripheral:(CBPeripheral *)peripheral error:(nullable NSError *)error {
    NSLog(@"didFailToConnectPeripheral: %@", peripheral);
}

- (void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)peripheral error:(nullable NSError *)error {
    NSLog(@"didDisconnectPeripheral: %@", peripheral);
    mPeripheral = nil;
    [self startScan];
}

// ================================================================================

- (void)peripheralDidUpdateName:(CBPeripheral *)peripheral NS_AVAILABLE(10_9, 6_0) {
    NSLog(@"peripheralDidUpdateName: %@", peripheral);
}

- (void)peripheralDidInvalidateServices:(CBPeripheral *)peripheral NS_DEPRECATED(NA, NA, 6_0, 7_0) {
    NSLog(@"peripheralDidInvalidateServices: %@", peripheral);
}

- (void)peripheral:(CBPeripheral *)peripheral didModifyServices:(NSArray<CBService *> *)invalidatedServices NS_AVAILABLE(10_9, 7_0) {
    NSLog(@"didModifyServices: %@", peripheral);
}

- (void)peripheralDidUpdateRSSI:(CBPeripheral *)peripheral error:(nullable NSError *)error {
    NSLog(@"peripheralDidUpdateRSSI: %@, error = %@", peripheral, error);
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(nullable NSError *)error {
    NSLog(@"didDiscoverServices: %@, error = %@", peripheral, error);
    for (CBService *service in peripheral.services) {
        NSLog(@"service = %@", service.UUID);
        [peripheral discoverCharacteristics:nil forService:service];
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverIncludedServicesForService:(CBService *)service error:(nullable NSError *)error {
    NSLog(@"didDiscoverIncludedServicesForService: %@, service = %@, error = %@", peripheral, service, error);
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(nullable NSError *)error {
    NSLog(@"didDiscoverCharacteristicsForService: %@, service = %@, error = %@", peripheral, service, error);
}

- (void)peripheral:(CBPeripheral *)peripheral didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic error:(nullable NSError *)error {
    if (error != nil) {
        NSLog(@"didUpdateValueForCharacteristic: %@, characteristic = %@, error = %@", peripheral, characteristic, error);
    }

    CavanBleChar *bleChar = [mDictChars objectForKey:characteristic.UUID];
    if (bleChar != nil) {
        [bleChar setReadStatus:error];
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didWriteValueForCharacteristic:(CBCharacteristic *)characteristic error:(nullable NSError *)error {
    if (error != nil) {
        NSLog(@"didWriteValueForCharacteristic: %@, characteristic = %@, error = %@", peripheral, characteristic, error);
    }

    CavanBleChar *bleChar = [mDictChars objectForKey:characteristic.UUID];
    if (bleChar != nil) {
        [bleChar setWriteStatus:error];
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic error:(nullable NSError *)error {
    if (error != nil) {
        NSLog(@"didUpdateNotificationStateForCharacteristic: %@, characteristic = %@, error = %@", peripheral, characteristic.UUID, error);
    }

    NSLog(@"isNotifying = %d", characteristic.isNotifying);
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverDescriptorsForCharacteristic:(CBCharacteristic *)characteristic error:(nullable NSError *)error {
    NSLog(@"didDiscoverDescriptorsForCharacteristic: %@, characteristic = %@, error = %@", peripheral, characteristic, error);
}

- (void)peripheral:(CBPeripheral *)peripheral didUpdateValueForDescriptor:(CBDescriptor *)descriptor error:(nullable NSError *)error {
    NSLog(@"didUpdateValueForDescriptor: %@, descriptor = %@, error = %@", peripheral, descriptor, error);
}

- (void)peripheral:(CBPeripheral *)peripheral didWriteValueForDescriptor:(CBDescriptor *)descriptor error:(nullable NSError *)error {
    NSLog(@"didWriteValueForDescriptor: %@, descriptor = %@, error = %@", peripheral, descriptor, error);
}

@end
