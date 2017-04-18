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

import com.cavan.java.CavanString;

public class CavanBleGatt extends CavanBluetoothAdapter {

	private static final int MAX_CONN_COUNT = 20;

	private static final long CONNECT_OVERTIME = 1000;
	private static final long DISCOVER_OVERTIME = 2000;
	private static final long CONNECT_WAIT_TIME = 1000;

	private static final int MSG_CONNECTION_STATE_CHANGE = 1;
	private static final int MSG_SERVICES_DISCOVERED = 2;
	private static final int MSG_CHARACTERISTIC_READ = 3;
	private static final int MSG_CHARACTERISTIC_WRITE = 4;
	private static final int MSG_CHARACTERISTIC_CHANGED = 5;
	private static final int MSG_DESCRIPTOR_READ = 6;
	private static final int MSG_DESCRIPTOR_WRITE = 7;
	private static final int MSG_RELIABLE_WRITE_COMPLETED = 8;
	private static final int MSG_READ_REMOTE_RSSI = 9;
	private static final int MSG_MTU_CHANGED = 10;
	private static final int MSG_CONNECT = 11;
	private static final int MSG_DISCONNECT = 12;
	private static final int MSG_DISCOVER_TIMEOUT = 13;

	public static final int PROPERTY_NOTIFY_ALL = BluetoothGattCharacteristic.PROPERTY_NOTIFY;
	public static final int PROPERTY_READ_ALL = BluetoothGattCharacteristic.PROPERTY_READ;
	public static final int PROPERTY_WRITE_ALL = BluetoothGattCharacteristic.PROPERTY_WRITE |
			BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE | BluetoothGattCharacteristic.PROPERTY_SIGNED_WRITE;

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

	public static class GattInvalidStateException extends Exception {

		private static final long serialVersionUID = 1L;

		public GattInvalidStateException(String message) {
			super(message);
		}
	}

	private int mGattState;
	private boolean mGattReady;

	private boolean mConnected;
	private boolean mConnEnabled;

	private int mGattTimeout;
	private int mGattConnTimes;
	private int mServiceConnTimes;
	private CavanBleGattEventListener mEventListener = sEventListenerDummy;

	private UUID mUuid;
	private BluetoothGatt mGatt;
	private BluetoothDevice mDevice;
	private BluetoothGattService mService;
	private CavanBleGattHandler mGattHandler = new CavanBleGattHandler();
	private HashMap<UUID, CavanBleChar> mCharMap = new HashMap<UUID, CavanBleChar>();

	private BluetoothGattCallback mCallback = new BluetoothGattCallback() {

		@Override
		public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
			if (checkGattValid(gatt)) {
				mGattHandler.sendMessage(MSG_CONNECTION_STATE_CHANGE, status, newState);
			}
		}

		@Override
		public void onServicesDiscovered(BluetoothGatt gatt, int status) {
			if (checkGattValid(gatt)) {
				mGattHandler.sendMessage(MSG_SERVICES_DISCOVERED, status);
			}
		}

		@Override
		public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
			if (checkGattValid(gatt)) {
				CavanBleGatt.this.onCharacteristicRead(characteristic, status);
			}
		}

		@Override
		public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
			if (checkGattValid(gatt)) {
				CavanBleGatt.this.onCharacteristicWrite(characteristic, status);
			}
		}

		@Override
		public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
			if (checkGattValid(gatt)) {
				CavanBleGatt.this.onCharacteristicChanged(characteristic);
			}
		}

		@Override
		public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
			if (checkGattValid(gatt)) {
				CavanBleGatt.this.onDescriptorRead(descriptor, status);
			}
		}

		@Override
		public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
			if (checkGattValid(gatt)) {
				CavanBleGatt.this.onDescriptorWrite(descriptor, status);
			}
		}

		@Override
		public void onReliableWriteCompleted(BluetoothGatt gatt, int status) {
			if (checkGattValid(gatt)) {
				CavanBleGatt.this.onReliableWriteCompleted(status);
			}
		}

		@Override
		public void onReadRemoteRssi(BluetoothGatt gatt, int rssi, int status) {
			if (checkGattValid(gatt)) {
				CavanBleGatt.this.onReadRemoteRssi(rssi, status);
			}
		}

		@Override
		public void onMtuChanged(BluetoothGatt gatt, int mtu, int status) {
			if (checkGattValid(gatt)) {
				CavanBleGatt.this.onMtuChanged(mtu, status);
			}
		}
	};

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
		disconnectGatt(true);
		mGattHandler.quit();
		super.finalize();
	}

	synchronized public void setEventListener(CavanBleGattEventListener listener) {
		if (listener != null) {
			mEventListener = listener;
		} else {
			mEventListener = sEventListenerDummy;
		}
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

		if (mConnEnabled && mConnected != connected) {
			mConnected = connected;
			onConnectionStateChanged(connected);
		} else {
			mConnected = false;
		}
	}

	synchronized private void setConnectFailed() {
		if (mConnEnabled) {
			disconnectGatt(true);
			onConnectFailed();
		}
	}

	synchronized public CavanBleChar openChar(UUID uuid) {
		if (mService == null) {
			CavanAndroid.dLog("mService == null");
			return null;
		}

		BluetoothGattCharacteristic characteristic = mService.getCharacteristic(uuid);
		if (characteristic == null) {
			CavanAndroid.eLog("Failed to getCharacteristic: " + uuid);
			return null;
		}

		CavanBleChar bleChar = new CavanBleChar(this, characteristic);

		synchronized (mCharMap) {
			mCharMap.put(uuid, bleChar);
		}

		return bleChar;
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

	public boolean checkGattConnected() throws GattInvalidStateException {
		if (isGattConnected()) {
			return true;
		}

		throw new GattInvalidStateException("gatt not connected");
	}

	synchronized public void setGattReady(boolean ready) {
		mGattReady = ready;
	}

	synchronized public boolean isGattReady() {
		return mGattReady && isGattConnected();
	}

	public boolean checkGattReady() throws GattInvalidStateException {
		if (isGattReady()) {
			return true;
		}

		throw new GattInvalidStateException("gatt not ready");
	}

	synchronized public boolean isConnEnabled() {
		return mConnEnabled;
	}

	synchronized public boolean isConnected() {
		return mConnected && isGattConnected();
	}

	synchronized public int getGattTimeout() {
		return mGattTimeout;
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

	synchronized public boolean writeCharacteristic(BluetoothGattCharacteristic characteristic) throws GattInvalidStateException {
		return checkGattReady() && mGatt.writeCharacteristic(characteristic);
	}

	synchronized public void onWriteTimeout(CavanBleChar cavanBleChar) {
		mGattTimeout++;
		CavanAndroid.dLog("mGattTimeoutCount = " + mGattTimeout);
	}

	synchronized public boolean readCharacteristic(BluetoothGattCharacteristic characteristic) throws GattInvalidStateException {
		return checkGattReady() && mGatt.readCharacteristic(characteristic);
	}

	synchronized public boolean writeDescriptor(BluetoothGattDescriptor descriptor) throws GattInvalidStateException {
		return checkGattConnected() && mGatt.writeDescriptor(descriptor);
	}

	synchronized public boolean readDescriptor(BluetoothGattDescriptor descriptor) throws GattInvalidStateException {
		return checkGattConnected() && mGatt.readDescriptor(descriptor);
	}

	synchronized public boolean setCharacteristicNotification(BluetoothGattCharacteristic characteristic, boolean enable) throws GattInvalidStateException {
		return checkGattConnected() && mGatt.setCharacteristicNotification(characteristic, enable);
	}

	// ================================================================================

	protected boolean doInitialize() {
		CavanAndroid.dLog("doInitialize");
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

	// ================================================================================

	public boolean checkGattValid(BluetoothGatt gatt) {
		if (gatt == mGatt) {
			return true;
		}

		CavanAndroid.dLog("Close invalid gatt: " + gatt);
		gatt.disconnect();
		gatt.close();

		return false;
	}

	synchronized protected void onConnectionStateChange(int status, int newState) {
		CavanAndroid.dLog("onConnectionStateChange: status = " + status + ", newState = " + newState);

		if (status != 0) {
			disconnectGatt(false);
		} else {
			mGattState = newState;

			if (newState == BluetoothProfile.STATE_CONNECTED) {
				mGattHandler.removeMessages(MSG_CONNECT);
				mGattConnTimes = 0;
				discoverServices();
			} else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
				setConnectStatus(false);
				mGattHandler.sendEmptyMessageDelayed(MSG_CONNECT, CONNECT_WAIT_TIME);
			}
		}
	}

	synchronized protected void onServicesDiscovered(int status) {
		CavanAndroid.dLog("onServicesDiscovered: status = " + status);

		if (status != 0) {
			disconnectGatt(false);
		} else if (isGattConnected()) {
			mGattHandler.removeMessages(MSG_DISCOVER_TIMEOUT);

			dumpServices();

			mService = mGatt.getService(mUuid);
			CavanAndroid.dLog("mService = " + mService);
			CavanAndroid.dLog("mServiceConnTimes = " + mServiceConnTimes);

			mGattTimeout = 0;

			if (mService != null && doInitialize() && onInitialize() && mGattTimeout == 0) {
				setConnectStatus(true);
				mServiceConnTimes = 0;
			} else if (++mServiceConnTimes > MAX_CONN_COUNT) {
				setConnectFailed();
			} else {
				disconnectGatt(false);
			}
		}
	}

	protected void onCharacteristicRead(BluetoothGattCharacteristic characteristic, int status) {
		synchronized (mCharMap) {
			CavanBleChar bleChar = mCharMap.get(characteristic.getUuid());
			if (bleChar != null) {
				bleChar.setReadStatus(status);
			}
		}
	}

	protected void onCharacteristicWrite(BluetoothGattCharacteristic characteristic, int status) {
		synchronized (mCharMap) {
			CavanBleChar bleChar = mCharMap.get(characteristic.getUuid());
			if (bleChar != null) {
				bleChar.setWriteStatus(status);
			}
		}
	}

	protected void onCharacteristicChanged(BluetoothGattCharacteristic characteristic) {
		synchronized (mCharMap) {
			CavanBleChar bleChar = mCharMap.get(characteristic.getUuid());
			if (bleChar != null) {
				bleChar.onDataAvailable();
			}
		}
	}

	protected void onDescriptorRead(BluetoothGattDescriptor descriptor, int status) {
		synchronized (mCharMap) {
			CavanBleChar bleChar = mCharMap.get(descriptor.getCharacteristic().getUuid());
			if (bleChar != null) {
				bleChar.setDescReadStatus(status);
			}
		}
	}

	protected void onDescriptorWrite(BluetoothGattDescriptor descriptor, int status) {
		CavanAndroid.dLog("onDescriptorWrite: descriptor = " + descriptor.getUuid() + ", status = " + status);

		synchronized (mCharMap) {
			CavanBleChar bleChar = mCharMap.get(descriptor.getCharacteristic().getUuid());
			if (bleChar != null) {
				bleChar.setDescWriteStatus(status);
			}
		}
	}

	protected void onReliableWriteCompleted(int status) {
		CavanAndroid.dLog("onReliableWriteCompleted: " + ", status = " + status);
	}

	protected void onReadRemoteRssi(int rssi, int status) {
		CavanAndroid.dLog("onReadRemoteRssi: rssi = " + rssi + ", status = " + status);
	}

	protected void onMtuChanged(int mtu, int status) {
		CavanAndroid.dLog("onMtuChanged: mtu = " + mtu + ", status = " + status);
	}

	// ================================================================================

	synchronized private boolean connectGatt() {
		mGattHandler.removeMessages(MSG_CONNECT);

		CavanAndroid.dLog("connectGatt: enable = " + mConnEnabled);
		CavanAndroid.dLog("mGattConnTimes = " + mGattConnTimes);

		if (++mGattConnTimes > MAX_CONN_COUNT) {
			return false;
		}

		if (!mConnEnabled) {
			return false;
		}

		if (!isPoweredOn()) {
			CavanAndroid.dLog("bluetooth not power on");
			return false;
		}

		if (isGattConnected()) {
			CavanAndroid.dLog("gatt is connected");
			return true;
		}

		CavanAndroid.dLog("device = " + mDevice);

		if (mDevice == null) {
			CavanAndroid.eLog("mDevice is null");
			return false;
		}

		mConnected = false;

		if (mGatt == null) {
			mGattState = BluetoothProfile.STATE_DISCONNECTED;

			mGatt = mDevice.connectGatt(mContext, false, mCallback);
			if (mGatt == null) {
				CavanAndroid.eLog("Failed to connectGatt");
				return false;
			}
		} else {
			mGatt.disconnect();

			if (!mGatt.connect()) {
				CavanAndroid.eLog("Failed to connect");
				return false;
			}
		}

		mGattHandler.sendEmptyMessageDelayed(MSG_CONNECT, CONNECT_OVERTIME);

		return true;
	}

	synchronized private void disconnectGatt(boolean cleanup) {
		mGattHandler.removeMessages(MSG_CONNECT);

		CavanAndroid.dLog("disconnectGatt: gatt = " + mGatt + ", cleanup = " + cleanup);
		CavanAndroid.dumpstack();

		if (cleanup) {
			mConnEnabled = false;
		}

		mGattState = BluetoothProfile.STATE_DISCONNECTED;
		mGattReady = false;
		mService = null;

		synchronized (mCharMap) {
			mCharMap.clear();
		}

		if (mGatt != null) {
			mGatt.disconnect();

			if (cleanup) {
				mGatt.close();
				mGatt = null;
			} else {
				mGattHandler.sendEmptyMessageDelayed(MSG_CONNECT, CONNECT_WAIT_TIME);
			}
		}
	}

	synchronized public boolean discoverServices() {
		mGattHandler.removeMessages(MSG_DISCOVER_TIMEOUT);

		if (isGattConnected()) {
			if (mGatt.discoverServices()) {
				mGattHandler.sendEmptyMessageDelayed(MSG_DISCOVER_TIMEOUT, DISCOVER_OVERTIME);
				return true;
			}

			disconnectGatt(false);
		}

		return false;
	}

	synchronized public boolean connect() {
		mGattConnTimes = 0;
		mServiceConnTimes = 0;
		mConnEnabled = true;
		return connectGatt();
	}

	synchronized public boolean connect(BluetoothDevice device) {
		mDevice = device;
		return connect();
	}

	synchronized public void disconnect() {
		disconnectGatt(true);
	}

	// ================================================================================

	public class CavanBleGattHandler extends CavanThreadedHandler {

		public CavanBleGattHandler() {
			super(CavanBleGattHandler.class);
		}

		@Override
		public void handleMessage(Message msg) {
			removeMessages(msg.what);

			CavanAndroid.dLog("CavanBleGattHandler: message = " + msg.what);

			if (!isConnEnabled()) {
				CavanAndroid.dLog("connect is disabled");
				return;
			}

			switch (msg.what) {
			case MSG_CONNECTION_STATE_CHANGE:
				onConnectionStateChange(msg.arg1, msg.arg2);
				break;

			case MSG_SERVICES_DISCOVERED:
				onServicesDiscovered(msg.arg1);
				break;

			case MSG_CHARACTERISTIC_READ:
				onCharacteristicRead((BluetoothGattCharacteristic) msg.obj, msg.arg1);
				break;

			case MSG_CHARACTERISTIC_WRITE:
				onCharacteristicWrite((BluetoothGattCharacteristic) msg.obj, msg.arg1);
				break;

			case MSG_CHARACTERISTIC_CHANGED:
				onCharacteristicChanged((BluetoothGattCharacteristic) msg.obj);
				break;

			case MSG_DESCRIPTOR_READ:
				onDescriptorRead((BluetoothGattDescriptor) msg.obj, msg.arg1);
				break;

			case MSG_DESCRIPTOR_WRITE:
				onDescriptorWrite((BluetoothGattDescriptor) msg.obj, msg.arg1);
				break;

			case MSG_RELIABLE_WRITE_COMPLETED:
				onReliableWriteCompleted(msg.arg1);
				break;

			case MSG_READ_REMOTE_RSSI:
				onReadRemoteRssi(msg.arg1, msg.arg2);
				break;

			case MSG_MTU_CHANGED:
				onMtuChanged(msg.arg1, msg.arg2);
				break;

			case MSG_CONNECT:
				CavanAndroid.dLog("MSG_CONNECT");
				if (!connectGatt()) {
					setConnectFailed();
				}
				break;

			case MSG_DISCONNECT:
				CavanAndroid.dLog("MSG_DISCONNECT");
				disconnectGatt(false);
				break;

			case MSG_DISCOVER_TIMEOUT:
				CavanAndroid.dLog("MSG_DISCOVER_TIMEOUT");
				disconnectGatt(false);
				break;
			}
		}
	}
}
