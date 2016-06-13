package com.cavan.cavanutils;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

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

	@Override
	public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
		CavanUtils.logE("onConnectionStateChange: " + status + " => " + newState);

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
		CavanUtils.logE("onServicesDiscovered: status = " + status);

		int serviceIndex = 0;

		for (BluetoothGattService service : mGatt.getServices()) {
			CavanUtils.logE((++serviceIndex) + ". service = " + service.getUuid());

			int characteristicIndex = 0;

			for (BluetoothGattCharacteristic characteristic : service.getCharacteristics()) {
				CavanUtils.logE("\t" + (++characteristicIndex) + ". characteristic = " + characteristic.getUuid());

				int descriptorIndex = 0;

				for (BluetoothGattDescriptor descriptor : characteristic.getDescriptors()) {
					CavanUtils.logE("\t\t" + (++descriptorIndex) + " .descriptor = " + descriptor.getUuid());
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
		CavanUtils.logE("onCharacteristicRead: characteristic = " + characteristic.getUuid() + ", status = " + status);

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
		CavanUtils.logE("onDescriptorRead: descriptor = " + descriptor.getUuid() + ", status = " + status);
		super.onDescriptorRead(gatt, descriptor, status);
	}

	@Override
	public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
		CavanUtils.logE("onDescriptorWrite: descriptor = " + descriptor.getUuid() + ", status = " + status);
		super.onDescriptorWrite(gatt, descriptor, status);
	}

	@Override
	public void onReliableWriteCompleted(BluetoothGatt gatt, int status) {
		CavanUtils.logE("onReliableWriteCompleted: " + ", status = " + status);
		super.onReliableWriteCompleted(gatt, status);
	}

	@Override
	public void onReadRemoteRssi(BluetoothGatt gatt, int rssi, int status) {
		CavanUtils.logE("onReadRemoteRssi: rssi = " + rssi + ", status = " + status);
		super.onReadRemoteRssi(gatt, rssi, status);
	}
}
