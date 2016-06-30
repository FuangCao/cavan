package com.cavan.android;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

import com.cavan.java.CavanJava;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.content.Context;

@SuppressLint("NewApi")
public abstract class CavanBleGatt extends BluetoothGattCallback {

	private UUID mUuid;
	private BluetoothGatt mGatt;
	private BluetoothDevice mDevice;
	private BluetoothGattService mService;
	private List<CavanBleChar> mReadChars = new ArrayList<CavanBleChar>();
	private List<CavanBleChar> mWriteChars = new ArrayList<CavanBleChar>();

	protected abstract boolean doInit();

	public CavanBleGatt(BluetoothDevice device, UUID uuid) {
		super();
		mDevice = device;
		mUuid = uuid;
	}

	protected void onConnected() {

	}

	protected void onDisconnected() {

	}

	protected void onDataReceived(CavanBleChar bleChar, byte[] data) {

	}

	private void onDataReceived(BluetoothGattCharacteristic characteristic) {
		for (CavanBleChar myChar : mReadChars) {
			if (myChar.match(characteristic)) {
				onDataReceived(myChar, characteristic.getValue());
				break;
			}
		}
	}

	synchronized public boolean connect(Context context) {
		mGatt = mDevice.connectGatt(context, false, this);
		if (mGatt == null) {
			return false;
		}

		return true;
	}

	synchronized private CavanBleChar openChar(UUID uuid, List<CavanBleChar> list) {
		if (mGatt == null || mService == null) {
			return null;
		}

		CavanBleChar characteristic = new CavanBleChar();
		if (characteristic.init(mGatt, mService, uuid)) {
			list.add(characteristic);
			return characteristic;
		}

		return null;
	}

	public CavanBleChar openReadChar(UUID uuid) {
		return openChar(uuid, mReadChars);
	}

	public CavanBleChar openWriteChar(UUID uuid) {
		return openChar(uuid, mWriteChars);
	}

	synchronized public void disconnect() {
		if (mGatt == null) {
			return;
		}

		mGatt.disconnect();
		mGatt.close();
		mGatt = null;
	}

	public static String getPropertyName(int property) {
		switch (property) {
		case BluetoothGattCharacteristic.PROPERTY_BROADCAST:
			return "BROADCAST";
		case BluetoothGattCharacteristic.PROPERTY_READ:
			return "READ";
		case BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE:
			return "WRITE_NO_RESPONSE";
		case BluetoothGattCharacteristic.PROPERTY_WRITE:
			return "WRITE";
		case BluetoothGattCharacteristic.PROPERTY_NOTIFY:
			return "NOTIFY";
		case BluetoothGattCharacteristic.PROPERTY_INDICATE:
			return "INDICATE";
		case BluetoothGattCharacteristic.PROPERTY_SIGNED_WRITE:
			return "SIGNED_WRITE";
		case BluetoothGattCharacteristic.PROPERTY_EXTENDED_PROPS:
			return "EXTENDED_PROPS";
		default:
			return null;
		}
	}

	public static String buildPropertiesString(int properties) {
		List<String> lines = new ArrayList<String>();
		for (int i = 0; i < 32; i++) {
			String property = getPropertyName(properties & (1 << i));
			if (property != null) {
				lines.add(property);
			}
		}

		return CavanJava.listToString(lines);
	}

	public static String getPermissionName(int permission) {
		switch (permission) {
		case BluetoothGattCharacteristic.PERMISSION_READ:
			return "READ";
		case BluetoothGattCharacteristic.PERMISSION_READ_ENCRYPTED:
			return "READ_ENCRYPTED";
		case BluetoothGattCharacteristic.PERMISSION_READ_ENCRYPTED_MITM:
			return "READ_ENCRYPTED_MITM";
		case BluetoothGattCharacteristic.PERMISSION_WRITE:
			return "WRITE";
		case BluetoothGattCharacteristic.PERMISSION_WRITE_ENCRYPTED:
			return "WRITE_ENCRYPTED";
		case BluetoothGattCharacteristic.PERMISSION_WRITE_ENCRYPTED_MITM:
			return "WRITE_ENCRYPTED_MITM";
		case BluetoothGattCharacteristic.PERMISSION_WRITE_SIGNED:
			return "WRITE_SIGNED";
		case BluetoothGattCharacteristic.PERMISSION_WRITE_SIGNED_MITM:
			return "WRITE_SIGNED_MITM";
		default:
			return null;
		}
	}

	public static String buildPermissionsString(int permissions) {
		List<String> lines = new ArrayList<String>();
		for (int i = 0; i < 32; i++) {
			String permission = getPermissionName(permissions & (1 << i));
			if (permission != null) {
				lines.add(permission);
			}
		}

		return CavanJava.listToString(lines);
	}

	@Override
	public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
		CavanAndroid.logE("onConnectionStateChange: " + status + " => " + newState);

		switch (newState) {
		case BluetoothProfile.STATE_CONNECTED:
			gatt.discoverServices();
			break;

		case BluetoothProfile.STATE_DISCONNECTED:
			disconnect();
			onDisconnected();
			break;
		}
	}

	@Override
	public void onServicesDiscovered(BluetoothGatt gatt, int status) {
		CavanAndroid.logE("onServicesDiscovered: status = " + status);

		int serviceIndex = 0;

		for (BluetoothGattService service : mGatt.getServices()) {
			CavanAndroid.logE((++serviceIndex) + ". service = " + service.getUuid());

			int characteristicIndex = 0;

			for (BluetoothGattCharacteristic characteristic : service.getCharacteristics()) {
				CavanAndroid.logE("\t" + (++characteristicIndex) + ". characteristic = " + characteristic.getUuid());

				int descriptorIndex = 0;

				String properties = buildPropertiesString(characteristic.getProperties());
				if (properties != null) {
					CavanAndroid.logE("\t\tproperties = " + properties);
				}

				String permissions = buildPermissionsString(characteristic.getPermissions());
				if (permissions != null) {
					CavanAndroid.logE("\t\tpermissions = " + permissions);
				}

				for (BluetoothGattDescriptor descriptor : characteristic.getDescriptors()) {
					CavanAndroid.logE("\t\t" + (++descriptorIndex) + " .descriptor = " + descriptor.getUuid());
				}
			}
		}

		mService = mGatt.getService(mUuid);
		if (mService != null && doInit()) {
			onConnected();
		} else {
			disconnect();
			onDisconnected();
		}

		super.onServicesDiscovered(gatt, status);
	}

	@Override
	public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
		CavanAndroid.logE("onCharacteristicRead: characteristic = " + characteristic.getUuid() + ", status = " + status);

		if (status == 0) {
			onDataReceived(characteristic);
		}

		super.onCharacteristicRead(gatt, characteristic, status);
	}

	@Override
	public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
		for (CavanBleChar bleChar : mWriteChars) {
			if (bleChar.setWriteStatus(characteristic, status)) {
				break;
			}
		}

		super.onCharacteristicWrite(gatt, characteristic, status);
	}

	@Override
	public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
		onDataReceived(characteristic);
		super.onCharacteristicChanged(gatt, characteristic);
	}

	@Override
	public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
		CavanAndroid.logE("onDescriptorRead: descriptor = " + descriptor.getUuid() + ", status = " + status);
		super.onDescriptorRead(gatt, descriptor, status);
	}

	@Override
	public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
		CavanAndroid.logE("onDescriptorWrite: descriptor = " + descriptor.getUuid() + ", status = " + status);
		super.onDescriptorWrite(gatt, descriptor, status);
	}

	@Override
	public void onReliableWriteCompleted(BluetoothGatt gatt, int status) {
		CavanAndroid.logE("onReliableWriteCompleted: " + ", status = " + status);
		super.onReliableWriteCompleted(gatt, status);
	}

	@Override
	public void onReadRemoteRssi(BluetoothGatt gatt, int rssi, int status) {
		CavanAndroid.logE("onReadRemoteRssi: rssi = " + rssi + ", status = " + status);
		super.onReadRemoteRssi(gatt, rssi, status);
	}
}
