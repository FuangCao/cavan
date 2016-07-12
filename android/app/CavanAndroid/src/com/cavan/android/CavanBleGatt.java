package com.cavan.android;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.UUID;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.content.Context;

import com.cavan.java.CavanJava;
import com.cavan.java.CavanProgressListener;

public class CavanBleGatt extends BluetoothGattCallback {

	public static final int FRAME_SIZE = 20;
	public static final long WRITE_TIMEOUT = 1000;
	public static final long COMMAND_TIMEOUT = 2000;

	public static final int PROPERTY_NOTIFY_ALL = BluetoothGattCharacteristic.PROPERTY_NOTIFY;
	public static final int PROPERTY_READ_ALL = BluetoothGattCharacteristic.PROPERTY_READ;
	public static final int PROPERTY_WRITE_ALL = BluetoothGattCharacteristic.PROPERTY_WRITE |
			BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE | BluetoothGattCharacteristic.PROPERTY_SIGNED_WRITE;
	public static final UUID CFG_UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");
	public static final UUID DESC_UUID = UUID.fromString("00002901-0000-1000-8000-00805f9b34fb");

	private int mGattState;
	private boolean mConnected;
	private boolean mDisconnectSend;
	private boolean mAutoConnectAllow;
	private boolean mAutoConnectEnable;
	private BleConnectThread mConnThread;

	private UUID mUuid;
	private Context mContext;
	private BluetoothGatt mGatt;
	private BluetoothDevice mDevice;
	private BluetoothGattService mService;
	private HashMap<BluetoothGattCharacteristic, CavanBleChar> mHashMapRead = new HashMap<BluetoothGattCharacteristic, CavanBleGatt.CavanBleChar>();
	private HashMap<BluetoothGattCharacteristic, CavanBleChar> mHashMapWrite = new HashMap<BluetoothGattCharacteristic, CavanBleGatt.CavanBleChar>();
	private HashMap<BluetoothGattCharacteristic, CavanBleChar> mHashMapNotify = new HashMap<BluetoothGattCharacteristic, CavanBleGatt.CavanBleChar>();

	protected boolean doInitialize() {
		CavanAndroid.logE("doInitialize");
		return true;
	}

	protected boolean onInitialized() {
		CavanAndroid.logE("onInitialized");
		return true;
	}

	protected void onConnectionStateChange(boolean connected) {
		CavanAndroid.logE("onConnectStatusChanged: connected = " + connected);
	}

	public interface CavanBleDataListener {
		void onDataReceived(byte[] data);
	}

	public CavanBleGatt(Context context, BluetoothDevice device, UUID uuid) {
		super();

		mUuid = uuid;
		mDevice = device;
		mContext = context;
	}

	@Override
	protected void finalize() throws Throwable {
		disconnect();
		super.finalize();
	}

	synchronized public void setAutoConnectEnable(boolean enable) {
		mAutoConnectEnable = enable;
	}

	synchronized public void setAutoConnectAllow(boolean allow) {
		mAutoConnectAllow = allow;
	}

	synchronized private void setConnectStatus(boolean connected) {
		if (connected) {
			mAutoConnectAllow = true;
			mDisconnectSend = false;

			if (mConnected) {
				return;
			}
		} else {
			if (mDisconnectSend) {
				return;
			}

			mDisconnectSend = true;
		}

		mConnected = connected;
		onConnectionStateChange(connected);
	}

	synchronized public CavanBleChar openChar(UUID uuid) {
		try {
			return new CavanBleChar(uuid);
		} catch (Exception e) {
			e.printStackTrace();
		}

		return null;
	}

	synchronized public boolean connect() {
		if (mGatt != null && isGattConnected()) {
			CavanAndroid.logE("GattConnected");
			return false;
		}

		mGatt = mDevice.connectGatt(mContext, false, this);

		return mGatt != null;
	}

	synchronized public boolean connect(boolean autoConnect) {
		mAutoConnectEnable = autoConnect;
		return connect();
	}

	synchronized private void disconnectInternal() {
		if (mGatt != null) {
			mGatt.disconnect();
			mGatt.close();
			mGatt = null;
		}
	}

	synchronized private boolean autoConnect() {
		disconnectInternal();

		if (mAutoConnectAllow && mAutoConnectEnable) {
			CavanAndroid.logE("Auto Connect");

			if (connect()) {
				return true;
			}
		}

		setConnectStatus(false);

		return false;
	}

	synchronized public void disconnect() {
		mAutoConnectEnable = false;

		disconnectInternal();
		setConnectStatus(false);
	}

	synchronized public boolean isGattConnected() {
		return mGattState == BluetoothProfile.STATE_CONNECTED;
	}

	synchronized public boolean isGattDisconnected() {
		return mGattState != BluetoothProfile.STATE_CONNECTED;
	}

	synchronized public boolean isConnected() {
		return mConnected;
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
		if (gatt != mGatt) {
			CavanAndroid.logE("Invalid gatt: " + gatt);
			return;
		}

		CavanAndroid.logE("onConnectionStateChange: " + status + " => " + newState);

		mGattState = newState;

		switch (newState) {
		case BluetoothProfile.STATE_CONNECTED:
			if (!gatt.discoverServices()) {
				CavanAndroid.logE("Failed to discoverServices");
				disconnectInternal();
			}
			break;

		case BluetoothProfile.STATE_DISCONNECTED:
			autoConnect();
			break;
		}
	}

	public void dumpServices() {
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
	}

	@Override
	public void onServicesDiscovered(BluetoothGatt gatt, int status) {
		CavanAndroid.logE("onServicesDiscovered: status = " + status);

		if (status == 0) {
			dumpServices();

			if (mConnThread == null) {
				mConnThread = new BleConnectThread();
				mConnThread.start();
			} else {
				mConnThread.setPending();
			}
		}

		super.onServicesDiscovered(gatt, status);
	}

	@Override
	public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
		CavanAndroid.logE("onDescriptorRead: characteristic = " + characteristic.getUuid() + ", status = " + status);
		CavanBleChar bleChar = mHashMapRead.get(characteristic);
		if (bleChar != null) {
			bleChar.setReadStatus(status);
		}

		super.onCharacteristicRead(gatt, characteristic, status);
	}

	@Override
	public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
		CavanBleChar bleChar = mHashMapWrite.get(characteristic);
		if (bleChar != null) {
			bleChar.setWriteStatus(status);
		}

		super.onCharacteristicWrite(gatt, characteristic, status);
	}

	@Override
	public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
		CavanBleChar bleChar = mHashMapNotify.get(characteristic);
		if (bleChar != null) {
			bleChar.onDataAvailable();
		}

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

	// ================================================================================

	private class BleConnectThread extends Thread {

		private boolean mPending = true;

		public void setPending() {
			mPending = true;
		}

		@Override
		public void run() {
			while (mPending) {
				mPending = false;

				if (mGatt == null || isGattDisconnected()) {
					break;
				}

				mService = mGatt.getService(mUuid);
				if (mService != null && doInitialize() && onInitialized()) {
					setConnectStatus(true);
				} else if (isGattConnected()) {
					autoConnect();
				}
			}

			mConnThread = null;
		}
	}

	// ================================================================================

	public class CavanBleChar {

		int mWriteStatus = -1;
		int mReadStatus = -1;
		private BluetoothGattCharacteristic mChar;
		private CavanBleDataListener mListener;
		private final CavanBleDataListener mListenerDefault = new CavanBleDataListener() {

			@Override
			public void onDataReceived(byte[] data) {
				CavanAndroid.logE("onDataReceived: length = " + data.length);
			}
		};

		public CavanBleChar(UUID uuid) throws Exception {
			super();

			synchronized (CavanBleGatt.this) {
				mListener = mListenerDefault;

				if (mGatt == null || mService == null) {
					throw new Exception("gatt not connect");
				}

				mChar = mService.getCharacteristic(uuid);
				if (mChar == null) {
					throw new Exception("uuid not found: " + uuid);
				}

				mGatt.setCharacteristicNotification(mChar, true);

				int properties = mChar.getProperties();

				if ((properties & PROPERTY_NOTIFY_ALL) != 0) {
					BluetoothGattDescriptor descriptor = mChar.getDescriptor(CFG_UUID);
					if (descriptor != null) {
						descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
						mGatt.writeDescriptor(descriptor);
					}

					mHashMapNotify.put(mChar, this);
				}

				if ((properties & PROPERTY_READ_ALL) != 0) {
					mHashMapRead.put(mChar, this);
				}

				if ((properties & PROPERTY_WRITE_ALL) != 0) {
					mHashMapWrite.put(mChar, this);
				}
			}
		}

		synchronized private boolean writeFrame(byte[] data, boolean sync) {
			if (mGatt == null) {
				CavanAndroid.logE("gatt not connect");
				return false;
			}

			if (!mChar.setValue(data)) {
				CavanAndroid.logE("Failed to setValue");
				return false;
			}

			if (sync) {
				mWriteStatus = -110;

				for (int i = 0; i < 5 && isGattConnected(); i++) {
					if (mGatt.writeCharacteristic(mChar)) {
						try {
							wait(WRITE_TIMEOUT);
						} catch (InterruptedException e) {
							e.printStackTrace();
						}

						if (mWriteStatus != -110) {
							return (mWriteStatus == 0);
						}

						CavanAndroid.logE("Failed to writeData" + i + ": status = " + mWriteStatus);
					} else {
						CavanAndroid.logE("Failed to writeCharacteristic" + i);

						if (isGattConnected()) {
							try {
								wait(WRITE_TIMEOUT);
							} catch (InterruptedException e) {
								e.printStackTrace();
							}
						} else {
							return false;
						}
					}
				}

				return false;
			} else {
				return mGatt.writeCharacteristic(mChar);
			}
		}

		public boolean writeData(byte[] data, boolean sync) {
			if (data.length > FRAME_SIZE) {
				int last, offset;
				byte[] block = new byte[FRAME_SIZE];

				for (offset = 0, last = data.length - FRAME_SIZE; offset <= last; offset += FRAME_SIZE) {
					CavanAndroid.ArrayCopy(data, offset, block, 0, FRAME_SIZE);
					if (!writeFrame(block, true)) {
						return false;
					}

					CavanAndroid.logE("writeData: " + (offset * 100 / data.length) + "%");
				}

				if (offset >= data.length) {
					return true;
				}

				data = CavanAndroid.ArrayCopy(data, offset, data.length - offset);
			}

			return writeFrame(data, sync);
		}

		public boolean writeData(byte[] data, CavanProgressListener listener) {
			listener.setValueRange(data.length);

			if (data.length > FRAME_SIZE) {
				int offset = 0;
				int last = data.length - FRAME_SIZE;
				byte[] block = new byte[FRAME_SIZE];

				while (offset <= last) {
					CavanAndroid.ArrayCopy(data, offset, block, 0, FRAME_SIZE);
					if (!writeFrame(block, true)) {
						return false;
					}

					offset += FRAME_SIZE;
					listener.setValue(offset);
				}

				if (offset < data.length) {
					data = CavanAndroid.ArrayCopy(data, offset, data.length - offset);
				} else {
					return true;
				}
			}

			if (!writeFrame(data, true)) {
				return false;
			}

			listener.finishValue();

			return true;
		}

		synchronized public boolean readCharacteristic() {
			return mGatt != null && mGatt.readCharacteristic(mChar);
		}

		synchronized public byte[] readData(long timeout) {
			mReadStatus = -110;

			if (!readCharacteristic()) {
				CavanAndroid.logE("Failed to sendReadCommand");
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

		synchronized public boolean isNotTimeout() {
			return mWriteStatus != -110 && mReadStatus != -110;
		}

		synchronized public boolean isTimeout() {
			return mWriteStatus == -110 || mReadStatus == -110;
		}

		synchronized public void setDataListener(CavanBleDataListener listener) {
			if (listener == null) {
				listener = mListenerDefault;
			}

			mListener = listener;
		}

		synchronized public final void onDataAvailable() {
			mListener.onDataReceived(mChar.getValue());
		}
	}
}
