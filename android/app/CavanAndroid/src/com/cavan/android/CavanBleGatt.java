package com.cavan.android;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.UUID;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.os.Message;

import com.cavan.java.CavanArray;
import com.cavan.java.CavanProgressListener;
import com.cavan.java.CavanString;

public class CavanBleGatt extends CavanBluetoothAdapter {

	public static final int FRAME_SIZE = 20;
	public static final int MAX_CONN_COUNT = 20;
	public static final long WRITE_CHAR_TIMEOUT = 3000;
	public static final long WRITE_DESC_TIMEOUT = 3000;
	public static final long COMMAND_TIMEOUT = 2000;

	public static final long CONNECT_OVERTIME = 1000;
	public static final long DISCOVER_OVERTIME = 2000;
	public static final long CONNECT_WAIT_TIME = 1000;
	public static final long DISCONNECT_WAIT_TIME = 3000;

	public static final int STATE_GATT_DISCONNECTED = 1;
	public static final int STATE_GATT_CONNECTED = 2;
	public static final int STATE_SERVICE_DISCOVERING = 3;
	public static final int STATE_SERVICE_DISCOVERED = 4;
	public static final int STATE_SERVICE_CONNECTED = 5;

	public static final int PROPERTY_NOTIFY_ALL = BluetoothGattCharacteristic.PROPERTY_NOTIFY;
	public static final int PROPERTY_READ_ALL = BluetoothGattCharacteristic.PROPERTY_READ;
	public static final int PROPERTY_WRITE_ALL = BluetoothGattCharacteristic.PROPERTY_WRITE |
			BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE | BluetoothGattCharacteristic.PROPERTY_SIGNED_WRITE;

	public static final UUID CFG_UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");
	public static final UUID DESC_UUID = UUID.fromString("00002901-0000-1000-8000-00805f9b34fb");

	public static final CavanBleGattEventListener sEventListenerDummy = new CavanBleGattEventListener() {

		@Override
		public boolean onInitialize() {
			CavanAndroid.dLog("onInitialize");
			return true;
		}

		@Override
		public void onConnectionStateChanged(boolean connected) {
			CavanAndroid.dLog("onConnectionStateChanged: connected = " + connected);
		}

		@Override
		public void onConnectFailed() {
			CavanAndroid.dLog("onConnectFailed");
		}

		@Override
		public void onBluetoothAdapterStateChanged(boolean enabled) {}
	};

	public interface CavanBleGattEventListener {
		boolean onInitialize();
		void onConnectFailed();
		void onConnectionStateChanged(boolean connected);
		void onBluetoothAdapterStateChanged(boolean enabled);
	}

	public interface CavanBleDataListener {
		void onDataReceived(byte[] data);
	}

	private int mGattState;
	private boolean mGattReady;
	private boolean mConnected;
	private BleConnHandlerThread mConnThread = new BleConnHandlerThread();

	private boolean mConnEnable;
	private int mGattTimeoutCount;
	private int mGattConnectCount;
	private int mServiceConnectCount;
	private CavanBleGattEventListener mEventListener = sEventListenerDummy;

	private UUID mUuid;
	private BluetoothGatt mGatt;
	private BluetoothDevice mDevice;
	private BluetoothGattService mService;
	private HashMap<UUID, CavanBleChar> mCharMap = new HashMap<UUID, CavanBleGatt.CavanBleChar>();
	private BluetoothGattCallback mCallback = new BluetoothGattCallback() {

		@Override
		public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
			CavanAndroid.dLog("onConnectionStateChange: status = " + status + ", state = " + newState);

			if (gatt != mGatt) {
				CavanAndroid.eLog("Invalid gatt: " + gatt);
				gatt.disconnect();
				gatt.close();
				return;
			}

			if (status != 0 && newState == BluetoothProfile.STATE_CONNECTED) {
				mGattState = BluetoothProfile.STATE_DISCONNECTED;
				disconnect(false);
			} else {
				mGattState = newState;

				if (newState == BluetoothProfile.STATE_CONNECTED) {
					mGattConnectCount = 0;

					if (mConnThread.getConnState() != STATE_SERVICE_CONNECTED) {
						mConnThread.setConnState(STATE_GATT_CONNECTED, CONNECT_WAIT_TIME);
					}
				} else {
					mConnThread.setConnState(STATE_GATT_DISCONNECTED, DISCONNECT_WAIT_TIME);
				}
			}
		}

		@Override
		public void onServicesDiscovered(BluetoothGatt gatt, int status) {
			CavanAndroid.dLog("onServicesDiscovered: status = " + status);

			if (status == 0) {
				if (mConnThread.getConnState() != STATE_SERVICE_CONNECTED) {
					mConnThread.setConnState(STATE_SERVICE_DISCOVERED, 0);
				}
			} else {
				disconnect(false);
			}

			super.onServicesDiscovered(gatt, status);
		}

		@Override
		public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
			// CavanAndroid.dLog("onCharacteristicRead: characteristic = " + characteristic.getUuid() + ", status = " + status);

			CavanBleChar bleChar = mCharMap.get(characteristic.getUuid());
			if (bleChar != null) {
				bleChar.setReadStatus(status);
			}

			super.onCharacteristicRead(gatt, characteristic, status);
		}

		@Override
		public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
			CavanBleChar bleChar = mCharMap.get(characteristic.getUuid());
			if (bleChar != null) {
				bleChar.setWriteStatus(status);
			}

			super.onCharacteristicWrite(gatt, characteristic, status);
		}

		@Override
		public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
			CavanBleChar bleChar = mCharMap.get(characteristic.getUuid());
			if (bleChar != null) {
				bleChar.onDataAvailable();
			}

			super.onCharacteristicChanged(gatt, characteristic);
		}

		@Override
		public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
			// CavanAndroid.dLog("onDescriptorRead: descriptor = " + descriptor.getUuid() + ", status = " + status);

			CavanBleChar bleChar = mCharMap.get(descriptor.getCharacteristic().getUuid());
			if (bleChar != null) {
				bleChar.setDescReadStatus(status);
			}

			super.onDescriptorRead(gatt, descriptor, status);
		}

		@Override
		public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
			CavanAndroid.dLog("onDescriptorWrite: descriptor = " + descriptor.getUuid() + ", status = " + status);

			CavanBleChar bleChar = mCharMap.get(descriptor.getCharacteristic().getUuid());
			if (bleChar != null) {
				bleChar.setDescWriteStatus(status);
			}

			super.onDescriptorWrite(gatt, descriptor, status);
		}

		@Override
		public void onReliableWriteCompleted(BluetoothGatt gatt, int status) {
			CavanAndroid.dLog("onReliableWriteCompleted: " + ", status = " + status);
			super.onReliableWriteCompleted(gatt, status);
		}

		@Override
		public void onReadRemoteRssi(BluetoothGatt gatt, int rssi, int status) {
			CavanAndroid.dLog("onReadRemoteRssi: rssi = " + rssi + ", status = " + status);
			super.onReadRemoteRssi(gatt, rssi, status);
		}
	};

	synchronized public void setEventListener(CavanBleGattEventListener listener) {
		if (listener != null) {
			mEventListener = listener;
		} else {
			mEventListener = sEventListenerDummy;
		}
	}

	protected boolean doInitialize() {
		CavanAndroid.dLog("doInitialize");

		setGattReady(true);

		return true;
	}

	protected boolean onInitialize() {
		return mEventListener.onInitialize();
	}

	protected void onConnectionStateChanged(boolean connected) {
		mEventListener.onConnectionStateChanged(connected);
	}

	protected void onConnectFailed() {
		mEventListener.onConnectFailed();
	}

	@Override
	protected void onBluetoothAdapterStateChanged(boolean enabled) {
		super.onBluetoothAdapterStateChanged(enabled);

		if (!enabled) {
			setConnectFailed();
		}

		mEventListener.onBluetoothAdapterStateChanged(enabled);
	}

	public CavanBleGatt(Context context, BluetoothDevice device, UUID uuid) {
		super(context);

		mUuid = uuid;
		mDevice = device;
		mAdapter = BluetoothAdapter.getDefaultAdapter();
	}

	public CavanBleGatt(Context context, UUID uuid) {
		this(context, null, uuid);
	}

	public CavanBleGatt(UUID uuid) {
		this(null, uuid);
	}

	@Override
	protected void finalize() throws Throwable {
		disconnect();

		if (mConnThread != null) {
			mConnThread.quit();
		}

		super.finalize();
	}

	public BluetoothDevice getDevice() {
		return mDevice;
	}

	public String getAddress() {
		if (mDevice == null) {
			return null;
		}

		return mDevice.getAddress();
	}

	synchronized private void setConnectStatus(boolean connected) {
		setGattReady(connected);

		if (mConnected != connected) {
			mConnected = connected;
			onConnectionStateChanged(connected);
		}
	}

	synchronized private void setConnectFailed() {
		if (mConnEnable) {
			disconnect();
			onConnectFailed();
		}
	}

	synchronized public CavanBleChar openChar(UUID uuid) {
		try {
			return new CavanBleChar(uuid);
		} catch (Exception e) {
			e.printStackTrace();
		}

		return null;
	}

	synchronized private boolean connectInternal() {
		CavanAndroid.dLog("connectInternal: enable = " + mConnEnable);

		if (!mConnEnable) {
			disconnect();
			return false;
		}

		mConnThread.updateConnState(CONNECT_OVERTIME);

		if (mGatt == null) {
			if (mDevice == null) {
				CavanAndroid.dLog("mDevice is null");
				return false;
			}

			CavanAndroid.dLog("connectGatt: device = " + mDevice);

			mGattState = BluetoothProfile.STATE_DISCONNECTED;

			mGatt = mDevice.connectGatt(mContext, true, mCallback);
			if (mGatt == null) {
				CavanAndroid.eLog("Failed to connectGatt");
				return false;
			}
		} else if (isGattConnected()) {
			CavanAndroid.dLog("gatt is connected");
			return true;
		} else {
			mGatt.disconnect();
			return  mGatt.connect();
		}

		return true;
	}

	synchronized public boolean connect() {
		mGattConnectCount = 0;
		mServiceConnectCount = 0;

		if (mConnEnable) {
			return true;
		}

		mConnEnable = isPoweredOn();

		return connectInternal();
	}

	synchronized public boolean connect(BluetoothDevice device) {
		if (mDevice != device) {
			mDevice = device;
			disconnect();
		}

		return connect();
	}

	synchronized public void disconnect(boolean cleanup) {
		CavanAndroid.dLog("disconnect: cleanup = " + cleanup);
		CavanAndroid.dumpstack();

		mGattReady = false;

		if (cleanup) {
			mConnEnable = false;
		}

		mConnThread.setConnState(STATE_GATT_DISCONNECTED, DISCONNECT_WAIT_TIME);

		mService = null;
		mCharMap.clear();

		CavanAndroid.dLog("mGatt = " + mGatt);

		if (mGatt != null) {
			mGatt.disconnect();

			if (cleanup) {
				mGatt.close();
				mGatt = null;
			}
		}
	}

	synchronized public void disconnect() {
		disconnect(true);
	}

	synchronized public int getGattState() {
		if (mGatt != null) {
			return mGattState;
		}

		return BluetoothProfile.STATE_DISCONNECTED;
	}

	synchronized public boolean isGattConnected() {
		return getGattState() == BluetoothProfile.STATE_CONNECTED;
	}

	synchronized public boolean isGattDisconnected() {
		return getGattState() != BluetoothProfile.STATE_CONNECTED;
	}

	synchronized public boolean isConnected() {
		return mConnected && isGattConnected();
	}

	synchronized public void setGattReady(boolean ready) {
		mGattReady = ready;
	}

	synchronized public boolean isGattReady() {
		return mGattReady && isGattConnected();
	}

	synchronized public boolean isConnectEnabled() {
		return mConnEnable;
	}

	synchronized public int getGattTimeoutCount() {
		return mGattTimeoutCount;
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

		return CavanString.fromList(lines);
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

		return CavanString.fromList(lines);
	}

	public void dumpServices() {
		int serviceIndex = 0;

		for (BluetoothGattService service : mGatt.getServices()) {
			CavanAndroid.dLog((++serviceIndex) + ". service = " + service.getUuid());

			int characteristicIndex = 0;

			for (BluetoothGattCharacteristic characteristic : service.getCharacteristics()) {
				CavanAndroid.dLog("\t" + (++characteristicIndex) + ". characteristic = " + characteristic.getUuid());

				int descriptorIndex = 0;

				String properties = buildPropertiesString(characteristic.getProperties());
				if (properties != null) {
					CavanAndroid.dLog("\t\tproperties = " + properties);
				}

				String permissions = buildPermissionsString(characteristic.getPermissions());
				if (permissions != null) {
					CavanAndroid.dLog("\t\tpermissions = " + permissions);
				}

				for (BluetoothGattDescriptor descriptor : characteristic.getDescriptors()) {
					CavanAndroid.dLog("\t\t" + (++descriptorIndex) + " .descriptor = " + descriptor.getUuid());
				}
			}
		}
	}

	// ================================================================================

	public class BleConnHandlerThread extends CavanThreadedHandler {

		private int mConnState;

		public BleConnHandlerThread() {
			super(BleConnHandlerThread.class);
		}

		private void updateConnState(long delay) {
			removeMessages(0);
			sendEmptyMessageDelayed(0, delay);
		}

		public void setConnState(int state, long delay) {
			mConnState = state;

			if (mConnEnable) {
				if (state == STATE_SERVICE_CONNECTED) {
					setConnectStatus(true);
				} else {
					setConnectStatus(false);
					updateConnState(delay);
				}
			}
		}

		public int getConnState() {
			return mConnState;
		}

		@Override
		public void handleMessage(Message msg) {
			CavanAndroid.dLog("handleMessage: mConnState = " + mConnState + ", mConnEnable = " + mConnEnable);

			removeMessages(0);

			if (!mConnEnable) {
				disconnect();
				return;
			}

			switch (mConnState) {
			case STATE_GATT_DISCONNECTED:
				CavanAndroid.dLog("STATE_GATT_DISCONNECTED: mGattConnectCount = " + mGattConnectCount);

				if (mGattConnectCount < MAX_CONN_COUNT && isPoweredOn()) {
					if (!connectInternal()) {
						CavanAndroid.eLog("Failed to connectInternal");
					}

					mGattConnectCount++;
				} else {
					setConnectFailed();
				}
				break;

			case STATE_GATT_CONNECTED:
				CavanAndroid.dLog("STATE_GATT_CONNECTED");

				if (mGatt != null && mGatt.discoverServices()) {
					setConnState(STATE_SERVICE_DISCOVERING, DISCOVER_OVERTIME);
				} else {
					disconnect(false);
				}
				break;

			case STATE_SERVICE_DISCOVERING:
				CavanAndroid.dLog("STATE_SERVICE_DISCOVERING");
				disconnect(false);
				break;

			case STATE_SERVICE_DISCOVERED:
				CavanAndroid.dLog("STATE_SERVICE_DISCOVERED");

				dumpServices();

				if (mGatt != null) {
					mService = mGatt.getService(mUuid);
					CavanAndroid.dLog("mService = " + mService);
					CavanAndroid.dLog("mServiceConnectCount = " + mServiceConnectCount);

					mGattTimeoutCount = 0;

					if (mService != null && doInitialize() && onInitialize() && mGattTimeoutCount == 0) {
						setConnState(STATE_SERVICE_CONNECTED, 0);
						mServiceConnectCount = 0;
					} else if (mServiceConnectCount < MAX_CONN_COUNT) {
						disconnect(false);
						mServiceConnectCount++;
					} else {
						setConnectFailed();
					}
				}
				break;

			case STATE_SERVICE_CONNECTED:
				CavanAndroid.dLog("STATE_SERVICE_CONNECTED");
				break;
			}
		}
	}

	// ================================================================================

	public class CavanBleChar {

		int mReadStatus;
		int mWriteStatus;
		int mDescReadStatus;
		int mDescWriteStatus;
		private BluetoothGattCharacteristic mChar;
		private CavanBleDataListener mListener;
		private final CavanBleDataListener mListenerDefault = new CavanBleDataListener() {

			@Override
			public void onDataReceived(byte[] data) {
				CavanAndroid.dLog("onDataReceived: length = " + data.length);
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

				mCharMap.put(mChar.getUuid(), this);
			}
		}

		synchronized public boolean writeCharacteristic(BluetoothGattCharacteristic characteristic) {
			return isGattReady() && mGatt.writeCharacteristic(characteristic);
		}

		synchronized private boolean writeFrame(byte[] data, boolean sync) {
			if (!mChar.setValue(data)) {
				CavanAndroid.eLog("Failed to setValue");
				return false;
			}

			if (sync) {
				mWriteStatus = -110;

				for (int i = 0; i < 3 && isGattReady(); i++) {
					if (writeCharacteristic(mChar)) {
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

						if (isGattReady()) {
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

				mGattTimeoutCount++;
				CavanAndroid.dLog("mGattTimeoutCount = " + mGattTimeoutCount);

				return false;
			} else {
				return writeCharacteristic(mChar);
			}
		}

		public boolean writeData(byte[] data, CavanProgressListener listener, boolean sync) {
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

		public boolean writeData(byte[] data, boolean sync) {
			return writeData(data, null, sync);
		}

		public boolean writeData(byte[] data, CavanProgressListener listener) {
			return writeData(data, listener, true);
		}

		public boolean writeData(byte[] data) {
			return writeData(data, true);
		}

		synchronized public boolean readCharacteristic() {
			return isGattReady() && mGatt.readCharacteristic(mChar);
		}

		synchronized public byte[] readData(long timeout) {
			mReadStatus = -110;

			if (!readCharacteristic()) {
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

		synchronized public boolean writeDescriptor(BluetoothGattDescriptor descriptor) {
			return isGattConnected() && mGatt.writeDescriptor(descriptor);
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

			if (!writeDescriptor(descriptor)) {
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

		synchronized public boolean readDescriptor(BluetoothGattDescriptor descriptor) {
			return isGattConnected() && mGatt.readDescriptor(descriptor);
		}

		synchronized public byte[] readDesc(BluetoothGattDescriptor descriptor, long timeout) {
			if (mGatt == null) {
				return null;
			}

			mDescReadStatus = -110;

			if (!readDescriptor(descriptor)) {
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

		synchronized public boolean setCharacteristicNotification(BluetoothGattCharacteristic characteristic, boolean enable) {
			return isGattConnected() && mGatt.setCharacteristicNotification(characteristic, enable);
		}

		synchronized public boolean setNotifyEnable(boolean enable) {
			if (mChar == null) {
				return false;
			}

			if (!setCharacteristicNotification(mChar, enable)) {
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

		synchronized int getDescWriteStatus() {
			return mDescWriteStatus;
		}

		synchronized void setDescReadStatus(int status) {
			mDescReadStatus = status;
			notify();
		}

		synchronized int getDescReadStatus() {
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
				listener = mListenerDefault;
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
}
