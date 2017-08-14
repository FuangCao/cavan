package com.cavan.android;

import java.util.UUID;
import java.util.concurrent.TimeoutException;

import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;

import com.cavan.android.CavanBleGatt.GattInvalidStateException;
import com.cavan.java.CavanArray;
import com.cavan.java.CavanProgressListener;

public class CavanBleChar {

	public static final UUID CFG_UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");
	public static final UUID DESC_UUID = UUID.fromString("00002901-0000-1000-8000-00805f9b34fb");

	private static final int FRAME_SIZE = 20;
	private static final long COMMAND_TIMEOUT = 2000;
	private static final long WRITE_CHAR_TIMEOUT = 3000;
	private static final long WRITE_DESC_TIMEOUT = 3000;

	public interface CavanBleDataListener {
		void onDataReceived(byte[] data);
	}

	private static final CavanBleDataListener mListenerDummy = new CavanBleDataListener() {

		@Override
		public void onDataReceived(byte[] data) {
			CavanAndroid.dLog("onDataReceived: length = " + data.length);
		}
	};

	private int mReadStatus;
	private int mWriteStatus;
	private int mDescReadStatus;
	private int mDescWriteStatus;

	private CavanBleGatt mBleGatt;
	private BluetoothGattCharacteristic mBleChar;
	private CavanBleDataListener mListener = mListenerDummy;

	public CavanBleChar(CavanBleGatt bleGatt, BluetoothGattCharacteristic bleChar) {
		mBleGatt = bleGatt;
		mBleChar = bleChar;
	}

	synchronized void setBleGatt(CavanBleGatt bleGatt) {
		mBleGatt = bleGatt;
	}

	synchronized CavanBleGatt getBleGatt() {
		return mBleGatt;
	}

	synchronized void setBleChar(BluetoothGattCharacteristic bleChar) {
		mBleChar = bleChar;
	}

	synchronized BluetoothGattCharacteristic getBleChar() {
		return mBleChar;
	}

	public boolean canRead() {
		return (mBleChar.getProperties() & BluetoothGattCharacteristic.PROPERTY_READ) != 0;
	}

	public boolean canWriteNoResponse() {
		return (mBleChar.getProperties() & BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE) != 0;
	}

	public boolean canWrite() {
		return (mBleChar.getProperties() & BluetoothGattCharacteristic.PROPERTY_WRITE) != 0;
	}

	public boolean canNotify() {
		return (mBleChar.getProperties() & BluetoothGattCharacteristic.PROPERTY_NOTIFY) != 0;
	}

	public boolean canIndicate() {
		return (mBleChar.getProperties() & BluetoothGattCharacteristic.PROPERTY_INDICATE) != 0;
	}

	synchronized private boolean writeFrame(byte[] data, boolean sync) throws TimeoutException, GattInvalidStateException {
		if (!mBleChar.setValue(data)) {
			CavanAndroid.eLog("Failed to setValue");
			return false;
		}

		if (sync) {
			mWriteStatus = -110;

			for (int i = 0; i < 3; i++) {
				if (mBleGatt.writeCharacteristic(mBleChar)) {
					try {
						wait(WRITE_CHAR_TIMEOUT);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}

					if (mWriteStatus != -110) {
						return (mWriteStatus == BluetoothGatt.GATT_SUCCESS);
					}

					CavanAndroid.eLog("Failed to writeCharacteristic" + i + ": status = " + mWriteStatus);
				} else {
					CavanAndroid.eLog("Failed to writeCharacteristic");

					if (i > 0) {
						break;
					}

					return false;
				}
			}

			mBleGatt.onWriteTimeout(this);

			throw new TimeoutException("writeCharacteristic timeout");
		} else {
			return mBleGatt.writeCharacteristic(mBleChar);
		}
	}

	synchronized public boolean writeData(byte[] data, CavanProgressListener listener, boolean sync) throws TimeoutException, GattInvalidStateException {
		if (listener != null) {
			listener.setValueRange(data.length);
			sync = true;
		}

		if (data.length > FRAME_SIZE) {
			int offset = 0;
			int last = data.length - FRAME_SIZE;
			byte[] block = new byte[FRAME_SIZE];

			while (offset <= last) {
				CavanArray.copy(data, offset, block, 0, FRAME_SIZE);
				if (!writeFrame(block, sync)) {
					return false;
				}

				offset += FRAME_SIZE;

				if (listener != null) {
					listener.setValue(offset);
				}
			}

			if (offset < data.length) {
				data = CavanArray.clone(data, offset, data.length - offset);
			} else {
				return true;
			}
		}

		if (!writeFrame(data, sync)) {
			return false;
		}

		if (listener != null) {
			listener.finishValue();
		}

		return true;
	}

	synchronized public boolean writeData(byte[] data, boolean sync) throws GattInvalidStateException, TimeoutException {
		return writeData(data, null, sync);
	}

	synchronized public boolean writeData(byte[] data, CavanProgressListener listener) throws GattInvalidStateException, TimeoutException {
		return writeData(data, listener, true);
	}

	synchronized public boolean writeData(byte[] data) throws GattInvalidStateException, TimeoutException {
		return writeData(data, true);
	}

	synchronized public byte[] readData(long timeout) throws GattInvalidStateException {
		mReadStatus = -110;

		if (!mBleGatt.readCharacteristic(mBleChar)) {
			CavanAndroid.eLog("Failed to readCharacteristic");
			return null;
		}

		try {
			if (timeout > 0) {
				wait(timeout);
			} else {
				wait();
			}
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		if (mReadStatus == BluetoothGatt.GATT_SUCCESS) {
			return mBleChar.getValue();
		}

		return null;
	}

	synchronized public boolean writeDesc(BluetoothGattDescriptor descriptor, byte[] value) throws GattInvalidStateException, TimeoutException {
		if (!descriptor.setValue(value)) {
			CavanAndroid.eLog("Failed to descriptor.setValue");
			return false;
		}

		mDescWriteStatus = -110;

		for (int i = 0; i < 3; i++) {
			if (mBleGatt.writeDescriptor(descriptor)) {
				try {
					wait(WRITE_DESC_TIMEOUT);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}

				if (mDescWriteStatus != -110) {
					return (mDescWriteStatus == BluetoothGatt.GATT_SUCCESS);
				}

				CavanAndroid.eLog("Failed to writeDescriptor" + i + ": status = " + mDescWriteStatus);
			} else {
				CavanAndroid.eLog("Failed to writeDescriptor");

				if (i > 0) {
					break;
				}

				return false;
			}
		}

		mBleGatt.onWriteTimeout(this);

		throw new TimeoutException("writeDescriptor timeout");
	}

	synchronized public boolean writeDesc(UUID uuid, byte[] value) throws GattInvalidStateException, TimeoutException {
		BluetoothGattDescriptor descriptor = mBleChar.getDescriptor(uuid);
		if (descriptor == null) {
			CavanAndroid.eLog("Failed to getDescriptor");
			return false;
		}

		return writeDesc(descriptor, value);
	}

	synchronized public byte[] readDesc(BluetoothGattDescriptor descriptor, long timeout) throws GattInvalidStateException {
		mDescReadStatus = -110;

		if (!mBleGatt.readDescriptor(descriptor)) {
			CavanAndroid.eLog("Failed to readDescriptor");
			return null;
		}

		try {
			wait(timeout);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		if (mDescReadStatus == BluetoothGatt.GATT_SUCCESS) {
			return descriptor.getValue();
		}

		return null;
	}

	synchronized public byte[] readDesc(UUID uuid, long timeout) throws GattInvalidStateException {
		BluetoothGattDescriptor descriptor = mBleChar.getDescriptor(uuid);
		if (descriptor == null) {
			return null;
		}

		return readDesc(descriptor, timeout);
	}

	synchronized public boolean setNotifyEnable(boolean enable) throws GattInvalidStateException, TimeoutException {
		if (!mBleGatt.setCharacteristicNotification(mBleChar, enable)) {
			CavanAndroid.eLog("Failed to setCharacteristicNotification");
			return false;
		}

		byte[] value;

		if (enable) {
			value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE;
		} else {
			value = BluetoothGattDescriptor.DISABLE_NOTIFICATION_VALUE;
		}

		return writeDesc(CFG_UUID, value);
	}

	synchronized public byte[] sendCommand(byte[] command) throws GattInvalidStateException, TimeoutException {
		if (writeData(command, true)) {
			return readData(COMMAND_TIMEOUT);
		}

		return null;
	}

	synchronized public void setWriteStatus(int status) {
		mWriteStatus = status;
		notify();
	}

	synchronized public int getWriteStatus() {
		return mWriteStatus;
	}

	synchronized public void setReadStatus(int status) {
		mReadStatus = status;
		notify();
	}

	synchronized public int getReadStatus() {
		return mReadStatus;
	}

	synchronized public void setDescWriteStatus(int status) {
		mDescWriteStatus = status;
		notify();
	}

	synchronized public int getDescWriteStatus() {
		return mDescWriteStatus;
	}

	synchronized public void setDescReadStatus(int status) {
		mDescReadStatus = status;
		notify();
	}

	synchronized public int getDescReadStatus() {
		return mDescReadStatus;
	}

	synchronized public boolean isNotTimeout() {
		return mWriteStatus != -110 && mReadStatus != -110;
	}

	synchronized public boolean isTimeout() {
		return mWriteStatus == -110 || mReadStatus == -110;
	}

	synchronized public boolean setDataListener(CavanBleDataListener listener) throws GattInvalidStateException, TimeoutException {
		boolean enable;

		if (listener == null) {
			listener = mListenerDummy;
			enable = false;
		} else {
			enable = true;
		}

		mListener = listener;

		return setNotifyEnable(enable);
	}

	synchronized public final void onDataAvailable() {
		mListener.onDataReceived(mBleChar.getValue());
	}
}
