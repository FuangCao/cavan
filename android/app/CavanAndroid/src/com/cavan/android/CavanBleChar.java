package com.cavan.android;

import java.util.UUID;

import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;

import com.cavan.java.CavanArray;
import com.cavan.java.CavanProgressListener;

public class CavanBleChar {

	public static final int FRAME_SIZE = 20;
	public static final long COMMAND_TIMEOUT = 2000;
	public static final long WRITE_CHAR_TIMEOUT = 3000;
	public static final long WRITE_DESC_TIMEOUT = 3000;

	public static final UUID CFG_UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");
	public static final UUID DESC_UUID = UUID.fromString("00002901-0000-1000-8000-00805f9b34fb");

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

	private CavanBleGatt mGatt;
	private BluetoothGattCharacteristic mChar;
	private CavanBleDataListener mListener = mListenerDummy;

	public CavanBleChar(CavanBleGatt bleGatt, BluetoothGattCharacteristic bleChar) {
		mGatt = bleGatt;
		mChar = bleChar;
	}

	synchronized void setBleGatt(CavanBleGatt bleGatt) {
		mGatt = bleGatt;
	}

	synchronized CavanBleGatt getBleGatt() {
		return mGatt;
	}

	synchronized void setBleChar(BluetoothGattCharacteristic bleChar) {
		mChar = bleChar;
	}

	synchronized BluetoothGattCharacteristic getBleChar() {
		return mChar;
	}

	synchronized private boolean writeFrame(byte[] data, boolean sync) {
		if (!mChar.setValue(data)) {
			CavanAndroid.eLog("Failed to setValue");
			return false;
		}

		if (sync) {
			mWriteStatus = -110;

			for (int i = 0; i < 3; i++) {
				if (mGatt.writeCharacteristic(mChar)) {
					try {
						wait(WRITE_CHAR_TIMEOUT);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}

					if (mWriteStatus != -110) {
						return (mWriteStatus == 0);
					}

					CavanAndroid.eLog("Failed to writeData" + i + ": status = " + mWriteStatus);
				} else {
					CavanAndroid.eLog("Failed to writeCharacteristic" + i);

					if (mGatt.isGattReady()) {
						try {
							wait(WRITE_CHAR_TIMEOUT);
						} catch (InterruptedException e) {
							e.printStackTrace();
						}
					} else {
						return false;
					}
				}
			}

			mGatt.onWriteTimeout(this);

			return false;
		} else {
			return mGatt.writeCharacteristic(mChar);
		}
	}

	synchronized public boolean writeData(byte[] data, CavanProgressListener listener, boolean sync) {
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

	synchronized public boolean writeData(byte[] data, boolean sync) {
		return writeData(data, null, sync);
	}

	synchronized public boolean writeData(byte[] data, CavanProgressListener listener) {
		return writeData(data, listener, true);
	}

	synchronized public boolean writeData(byte[] data) {
		return writeData(data, true);
	}

	synchronized public byte[] readData(long timeout) {
		mReadStatus = -110;

		if (!mGatt.readCharacteristic(mChar)) {
			CavanAndroid.eLog("Failed to sendReadCommand");
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

		if (mReadStatus != 0) {
			return null;
		}

		return mChar.getValue();
	}

	synchronized public boolean writeDesc(BluetoothGattDescriptor descriptor, byte[] value) {
		if (mGatt == null) {
			return false;
		}

		if (!descriptor.setValue(value)) {
			CavanAndroid.eLog("Failed to descriptor.setValue");
			return false;
		}

		mDescWriteStatus = -110;

		if (!mGatt.writeDescriptor(descriptor)) {
			CavanAndroid.eLog("Failed to writeDescriptor");
			return false;
		}

		try {
			wait(WRITE_DESC_TIMEOUT);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		return (mDescWriteStatus == 0);
	}

	synchronized public boolean writeDesc(UUID uuid, byte[] value) {
		if (mChar == null) {
			return false;
		}

		BluetoothGattDescriptor descriptor = mChar.getDescriptor(uuid);
		if (descriptor == null) {
			return false;
		}

		return writeDesc(descriptor, value);
	}

	synchronized public byte[] readDesc(BluetoothGattDescriptor descriptor, long timeout) {
		mDescReadStatus = -110;

		if (!mGatt.readDescriptor(descriptor)) {
			CavanAndroid.eLog("Failed to readDescriptor");
			return null;
		}

		try {
			wait(timeout);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		if (mDescReadStatus == 0) {
			return descriptor.getValue();
		}

		return null;
	}

	synchronized public byte[] readDesc(UUID uuid, long timeout) {
		if (mChar == null) {
			return null;
		}

		BluetoothGattDescriptor descriptor = mChar.getDescriptor(uuid);
		if (descriptor == null) {
			return null;
		}

		return readDesc(descriptor, timeout);
	}

	synchronized public boolean setNotifyEnable(boolean enable) {
		if (mChar == null) {
			return false;
		}

		if (!mGatt.setCharacteristicNotification(mChar, enable)) {
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

	synchronized public byte[] sendCommand(byte[] command) {
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

	synchronized public boolean setDataListener(CavanBleDataListener listener) {
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
		mListener.onDataReceived(mChar.getValue());
	}
}
