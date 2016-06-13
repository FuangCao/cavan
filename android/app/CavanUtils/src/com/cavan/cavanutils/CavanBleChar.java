package com.cavan.cavanutils;

import java.util.UUID;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;

@SuppressLint("NewApi")
public class CavanBleChar {

	public static final UUID CFG_UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");
	public static final UUID DESC_UUID = UUID.fromString("00002901-0000-1000-8000-00805f9b34fb");

	int mWriteStatus = -1;
	private BluetoothGatt mGatt;
	private BluetoothGattCharacteristic mChar;

	public boolean init(BluetoothGatt gatt, BluetoothGattService service, UUID uuid) {
		mGatt = gatt;
		mChar = service.getCharacteristic(uuid);
		if (mChar == null) {
			return false;
		}

		mGatt.setCharacteristicNotification(mChar, true);

		BluetoothGattDescriptor descriptor = mChar.getDescriptor(CFG_UUID);
		if (descriptor != null) {
			descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
			mGatt.writeDescriptor(descriptor);
		}

		return true;
	}

	synchronized public boolean writeData(byte[] data, boolean sync) {
		if (mChar == null) {
			CavanUtils.logE("mCharacteristic is null");
			return false;
		}

		if (!mChar.setValue(data)) {
			CavanUtils.logE("Failed to setValue");
			return false;
		}

		if (sync) {
			mWriteStatus = -110;

			for (int i = 0; i < 10; i++) {
				if (!mGatt.writeCharacteristic(mChar)) {
					CavanUtils.logE("Failed to writeCharacteristic");
					return false;
				}

				try {
					wait(2000);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}

				if (mWriteStatus == 0) {
					return true;
				}

				CavanUtils.logE("Failed to writeData" + i + ": status = " + mWriteStatus);
			}

			return false;
		} else {
			return mGatt.writeCharacteristic(mChar);
		}
	}

	public boolean writeData(byte[] data, int blkSize) {
		if (data.length > blkSize) {
			int last, offset;
			byte[] block = new byte[blkSize];

			for (offset = 0, last = data.length - blkSize; offset <= last; offset += blkSize) {
				CavanUtils.ArrayCopy(data, offset, block, 0, blkSize);
				if (!writeData(block, true)) {
					return false;
				}
			}

			if (offset >= data.length) {
				return true;
			}

			data = CavanUtils.ArrayCopy(data, offset, data.length - offset);
		}

		return writeData(data, true);
	}

	synchronized public void setWriteStatus(int status) {
		mWriteStatus = status;
		notify();
	}

	public boolean match(BluetoothGattCharacteristic characteristic) {
		return mChar == characteristic;
	}

	public boolean setWriteStatus(BluetoothGattCharacteristic characteristic, int status) {
		if (match(characteristic)) {
			setWriteStatus(status);
			return true;
		}

		return false;
	}
}
