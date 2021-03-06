//
//  CavanBleGatt.m
//  TestBle
//
//  Created by 曹福昂 on 16/7/7.
//  Copyright © 2016年 曹福昂. All rights reserved.
//

#import "CavanBleGatt.h"

@implementation CavanBleGatt

@synthesize rssi = mRssi;
@synthesize service = mService;
@synthesize peripheral = mPeripheral;
@synthesize identifier = mIdentifier;

// ================================================================================

- (CavanBleGatt *)initWithNames:(NSArray *)names
                          uuid:(CBUUID *)uuids {
    mQueue = dispatch_queue_create("com.cavan.bluetooth.gatt", DISPATCH_QUEUE_SERIAL);

    if (self = [super initWithDelegate:self queue:mQueue]) {
        mNames = names;
        mUUID = uuids;
        mDictChars = [NSMutableDictionary new];
    }

    return self;
}

- (BOOL)isReady {
    return mReady;
}

- (BOOL)isConnected {
    return mConnected;
}

- (BOOL)isPoweredOn {
    return mPoweredOn;
}

- (BOOL)isConnEnabled {
    return mConnEnable;
}

- (void)setConnEnable:(BOOL)enable {
    mConnEnable = enable;
}

- (void)startScan {
    if (mConnEnable && mPoweredOn) {
        mPeripheral = nil;
        [self scanForPeripheralsWithServices:nil options:nil];
    }
}

- (void)connectByIdentify:(NSUUID *)identifier {
    mConnEnable = true;
    mIdentifier = identifier;

    if (!mConnected) {
        [self stopScan];
        [self startScan];
    }
}

- (void)connect {
    [self connectByIdentify:nil];
}

- (void)connectByPeripheral:(CBPeripheral *)peripheral {
    mConnEnable = true;

    if (mConnected) {
        if (mPeripheral == peripheral) {
            return;
        }

        [self disconnectInternal];
    }

    mPeripheral = peripheral;

    [self stopScan];
    [self startConnThread:false];
}

- (void)addBleChar:(CavanBleChar *)bleChar
          withUUID:(CBUUID *)uuid {
    [mDictChars setObject:bleChar forKey:uuid];
}

- (CavanBleChar *)createBleChar:(CBCharacteristic *)characteristic {
    CavanBleChar *bleChar = [[CavanBleChar alloc] initWithCharacteristic:characteristic gatt:self];

    [self addBleChar:bleChar withUUID:characteristic.UUID];

    return bleChar;
}

- (void)disconnectInternal {
    if (mPeripheral != nil) {
        [self cancelPeripheralConnection:mPeripheral];
        mPeripheral = nil;
    }
}

- (void)disconnect {
    mConnEnable = false;
    mInitPending = false;
    mConnPending = false;

    [self stopScan];
    [self disconnectInternal];
}

- (void)onConnectStateChanged:(BOOL)connected {
    NSLog(@"onConnectStateChanged: connected = %d", connected);
}

- (BOOL)doInitialize {
    NSLog(@"doInitialize");
    return true;
}

- (void)setConnectState:(BOOL)connected {
    mReady = connected;

    if (mConnected != connected) {
        mConnected = connected;
        [self onConnectStateChanged:connected];
    }
}

- (void)initThread:(CBService *)service {
    NSLog(@"Enter: initThread");

    if (mInitRunning) {
        return;
    }

    mInitRunning = true;

    while (mInitPending && mConnEnable) {
        mInitPending = false;

        mService = service;
        mReady = true;

        if ([self doInitialize]) {
            [self setConnectState:true];
        } else {
            [self disconnectInternal];
        }
    };

    mInitRunning = false;

    NSLog(@"Exit: initThread");
}

- (void)connThread:(NSNumber *)needSleep {
    NSLog(@"Enter: connThread");

    if (mConnRunning) {
        return;
    }

    mConnRunning = true;

    if (needSleep.boolValue) {
        [NSThread sleepForTimeInterval:CAVAN_BLE_SCAN_TIME];
    }

    while (mConnPending && mPeripheral && mConnEnable) {
        NSLog(@"stopScan");
        [self stopScan];

        NSLog(@"cancelPeripheralConnection");
        [self cancelPeripheralConnection:mPeripheral];
        [self setConnectState:false];

        NSLog(@"connectPeripheral");
        [self connectPeripheral:mPeripheral options:nil];

        [NSThread sleepForTimeInterval:CAVAN_BLE_CONN_TIME];
    }

    mConnRunning = false;

    NSLog(@"Exit: connThread");
}

- (void)startConnThread:(BOOL)needSleep {
    mConnPending = true;

    if (!mConnRunning) {
        [NSThread detachNewThreadSelector:@selector(connThread:) toTarget:self withObject:[NSNumber numberWithBool:needSleep]];
    }
}

- (BOOL)isValidPeripheral:(CBPeripheral *)peripheral {
    if (mNames == nil) {
        return true;
    }

    for (NSString *node in mNames) {
        if ([node isEqualToString:peripheral.name]) {
            return true;
        }
    }

    return false;
}

- (BOOL)isValidService:(CBService *)service {
    return mUUID == nil || [mUUID isEqual:service.UUID];
}

// ================================================================================

- (void)centralManagerDidUpdateState:(CBCentralManager *)central {
    NSString *strState;

    switch (central.state) {
        case CBCentralManagerStateUnknown:
            strState = @"CBCentralManagerStateUnknown";
            break;

        case CBCentralManagerStateResetting:
            strState = @"CBCentralManagerStateResetting";
            break;

        case CBCentralManagerStateUnsupported:
            strState = @"CBCentralManagerStateUnsupported";
            break;

        case CBCentralManagerStateUnauthorized:
            strState = @"CBCentralManagerStateUnauthorized";
            break;

        case CBCentralManagerStatePoweredOff:
            strState = @"CBCentralManagerStatePoweredOff";
            mPoweredOn = false;
            break;

        case CBCentralManagerStatePoweredOn:
            strState = @"CBCentralManagerStatePoweredOn";
            mPoweredOn = true;
            [self startScan];
            break;

        default:
            strState = @"Unknown";
    }

    NSLog(@"centralManagerDidUpdateState: %@", strState);
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
    NSLog(@"didDiscoverPeripheral: %@, rssi = %@", peripheral, RSSI);
    if ([self isValidPeripheral:peripheral]) {
        if (mIdentifier != nil) {
            if ([mIdentifier isEqual:peripheral.identifier]) {
                [self connectByPeripheral:peripheral];
            }
        } else {
            if (mPeripheral == nil || RSSI.intValue > mRssi.intValue) {
                NSLog(@"Modify peripheral: %@ => %@", mPeripheral, peripheral);
                mPeripheral = peripheral;
                mRssi = RSSI;
            }

            [self startConnThread:true];
        }
    }
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral {
    NSLog(@"didConnectPeripheral: %@", peripheral);
    peripheral.delegate = self;
    [peripheral discoverServices:nil];
}

- (void)centralManager:(CBCentralManager *)central didFailToConnectPeripheral:(CBPeripheral *)peripheral error:(nullable NSError *)error {
    NSLog(@"didFailToConnectPeripheral: %@", peripheral);
    [self startScan];
}

- (void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)peripheral error:(nullable NSError *)error {
    NSLog(@"didDisconnectPeripheral: %@", peripheral);

    [self setConnectState:false];

    if (mPeripheral != nil) {
        [self startConnThread:false];
    } else {
        [self startScan];
    }
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

    mConnPending = false;

    for (CBService *service in peripheral.services) {
        NSLog(@"service = %@", service.UUID);
        if ([self isValidService:service]) {
            [peripheral discoverCharacteristics:nil forService:service];
        }
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverIncludedServicesForService:(CBService *)service error:(nullable NSError *)error {
    NSLog(@"didDiscoverIncludedServicesForService: %@, service = %@, error = %@", peripheral, service, error);
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(nullable NSError *)error {
    NSLog(@"didDiscoverCharacteristicsForService: %@, service = %@, error = %@", peripheral, service, error);
    if (error == nil) {
        mInitPending = true;

        if (!mInitRunning) {
            [NSThread detachNewThreadSelector:@selector(initThread:) toTarget:self withObject:service];
        }
    }
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
        NSLog(@"didUpdateNotificationStateForCharacteristic: %@, characteristic = %@, error = %@", peripheral, characteristic, error);
    }

    NSLog(@"uuid = %@, isNotifying = %d", characteristic.UUID, characteristic.isNotifying);
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
