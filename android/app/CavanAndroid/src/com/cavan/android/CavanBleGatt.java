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

	private static final int MAX_INIT_TIMES = 20;
	private static final int MAX_CONN_TIMES = 5;
	private static final int MAX_DISCONN_TIMES = 5;

	private static final long CONNECT_OVERTIME = 5000;
	private static final long DISCOVER_OVERTIME = 2000;
	private static final long CONNECT_WAIT_TIME = 1000;
	private static final long DISCONN_WAIT_TIME = 2000;

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
	private static final int MSG_DISCONN_WAIT = 12;
	private static final int MSG_CONNECT_TIMEOUT = 13;
	private static final int MSG_DISCOVER_TIMEOUT = 14;

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

	private int mConnTimes;
	private int mInitTimes;
	private int mDisconnTimes;
	private int mGattTimeout;
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
				// CavanBleGatt.this.onCharacteristicChanged(characteristic);
				mGattHandler.sendMessage(MSG_CHARACTERISTIC_CHANGED, characteristic);
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

	@Override
	protected void finalize() throws Throwable {
		disconnectGatt(true, true);
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

	synchronized public BluetoothDevice getDevice() {
		return mDevice;
	}

	synchronized public String getAddress() {
		if (mDevice == null) {
			return null;
		}

		return mDevice.getAddress();
	}

	synchronized private void setConnectStatus(boolean connected) {
		setGattReady(connected);

		if (mConnEnabled) {
			if (mConnected != connected) {
				mConnected = connected;
				onConnectionStateChanged(connected);
			}

			if (!connected) {
				mGattHandler.sendConnectMessage();
			}
		} else {
			mConnected = false;
		}
	}

	synchronized private void setConnectFailed() {
		if (mConnEnabled) {
			disconnect();
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
		return mConnected && isGattReady();
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

	synchronized protected void onGattConnected() {
		CavanAndroid.dLog("onGattConnected: mConnTimes = " + mConnTimes);

		mConnTimes = 0;
		mGattHandler.removeConnectMessages();

		if (mConnEnabled) {
			discoverServices();
		} else {
			disconnectGatt();
		}
	}

	synchronized protected void onGattDisconnected() {
		CavanAndroid.dLog("onGattDisconnected");

		setConnectStatus(false);
	}

	protected void onConnectTimeout() {
		CavanAndroid.dLog("onConnectTimeout: mConnTimes = " + mConnTimes);

		if (++mConnTimes > MAX_CONN_TIMES) {
			setConnectFailed();
		} else {
			disconnectGatt();
		}
	}

	protected void onDiscoverTimeout() {
		CavanAndroid.dLog("onDiscoverTimeout");
		disconnectGatt();
	}

	protected void onDisconnectWait() {
		CavanAndroid.dLog("onDisconnectWait: mDisconnTimes = " + mDisconnTimes);

		if (!isConnEnabled()) {
			if (++mDisconnTimes > MAX_DISCONN_TIMES) {
				disconnectGatt(true, true);
			} else {
				disconnectGatt();
			}
		}
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
			if (newState != BluetoothProfile.STATE_DISCONNECTED) {
				disconnectGatt();
			}
		} else {
			mGattState = newState;
		}

		if (mGattState == BluetoothProfile.STATE_CONNECTED) {
			onGattConnected();
		} else if (mGattState == BluetoothProfile.STATE_DISCONNECTED) {
			onGattDisconnected();
		}
	}

	synchronized protected void onServicesDiscovered(int status) {
		CavanAndroid.dLog("onServicesDiscovered: status = " + status);

		if (status == 0 && mConnEnabled && isGattConnected()) {
			mGattHandler.removeConnectMessages();

			dumpServices();

			mService = mGatt.getService(mUuid);
			CavanAndroid.dLog("mService = " + mService);
			CavanAndroid.dLog("mInitTimes = " + mInitTimes);

			mGattTimeout = 0;

			if (mService != null && doInitialize() && onInitialize() && mGattTimeout == 0) {
				setConnectStatus(true);
				mInitTimes = 0;
			} else if (++mInitTimes > MAX_INIT_TIMES) {
				setConnectFailed();
			} else {
				disconnectGatt();
			}
		} else {
			disconnectGatt();
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
		mGattHandler.removeConnectMessages();

		CavanAndroid.dLog("connectGatt: enable = " + mConnEnabled);

		if (mDevice == null) {
			CavanAndroid.eLog("mDevice is null");
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

		mConnected = false;
		mGattHandler.sendConnectTimeoutMessage();

		CavanAndroid.dLog("gatt = " + mGatt);

		if (mGatt == null) {
			mGattState = BluetoothProfile.STATE_DISCONNECTED;

			mGatt = mDevice.connectGatt(mContext, true, mCallback);
			if (mGatt == null) {
				CavanAndroid.eLog("Failed to connectGatt");
				return false;
			}
		} else {
			if (!mGatt.connect()) {
				CavanAndroid.eLog("Failed to connect");
				return false;
			}
		}

		return true;
	}

	synchronized private void disconnectGatt(boolean closeGatt, boolean disableAutoConn) {
		CavanAndroid.dLog("disconnectGatt: closeGatt = " + closeGatt + ", disableAutoConn = " + disableAutoConn);
		CavanAndroid.dumpstack();

		mGattHandler.removeConnectMessages();

		if (disableAutoConn) {
			mConnEnabled = false;
		}

		mGattState = BluetoothProfile.STATE_DISCONNECTED;
		mGattReady = false;
		mService = null;

		synchronized (mCharMap) {
			mCharMap.clear();
		}

		CavanAndroid.dLog("gatt = " + mGatt);

		if (mGatt != null) {
			mGatt.disconnect();

			if (closeGatt) {
				mGatt.close();
				mGatt = null;
			} else if (!mConnEnabled) {
				mGattHandler.sendDisconnWaitMessage();
			}
		}

		setConnectStatus(false);
	}

	synchronized private void disconnectGatt(boolean disableAutoConn) {
		disconnectGatt(false, disableAutoConn);
	}

	synchronized private void disconnectGatt() {
		disconnectGatt(false);
	}

	synchronized public boolean discoverServices() {
		mGattHandler.removeConnectMessages();

		if (isGattConnected()) {
			mGattHandler.sendDiscoverTimeoutMessage();

			if (mGatt.discoverServices()) {
				return true;
			}

			disconnectGatt();
		}

		return false;
	}

	synchronized public boolean connect() {
		mConnTimes = 0;
		mInitTimes = 0;
		mDisconnTimes = 0;
		mConnEnabled = true;
		return connectGatt();
	}

	synchronized public boolean setDevice(BluetoothDevice device) {
		if (mDevice == device) {
			return false;
		}

		if (mDevice != null && device != null) {
			String address = mDevice.getAddress();

			if (address != null && address.equalsIgnoreCase(device.getAddress())) {
				return false;
			}
		}

		disconnectGatt(true, true);
		mDevice = device;

		return true;
	}

	synchronized public boolean connect(BluetoothDevice device) {
		setDevice(device);
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

		public void removeConnectMessages() {
			removeMessages(MSG_CONNECT);
			removeMessages(MSG_DISCONN_WAIT);
			removeMessages(MSG_CONNECT_TIMEOUT);
			removeMessages(MSG_DISCOVER_TIMEOUT);
		}

		public void sendConnectMessage() {
			removeConnectMessages();

			if (isConnEnabled()) {
				CavanAndroid.dLog("sendEmptyMessageDelayed: MSG_CONNECT");
				sendEmptyMessageDelayed(MSG_CONNECT, CONNECT_WAIT_TIME);
			}
		}

		public void sendConnectTimeoutMessage() {
			removeConnectMessages();

			if (isConnEnabled()) {
				CavanAndroid.dLog("sendEmptyMessageDelayed: MSG_CONNECT_TIMEOUT");
				sendEmptyMessageDelayed(MSG_CONNECT_TIMEOUT, CONNECT_OVERTIME);
			}
		}

		public void sendDiscoverTimeoutMessage() {
			removeConnectMessages();

			if (isConnEnabled()) {
				CavanAndroid.dLog("sendEmptyMessageDelayed: MSG_DISCOVER_TIMEOUT");
				sendEmptyMessageDelayed(MSG_DISCOVER_TIMEOUT, DISCOVER_OVERTIME);
			}
		}

		public void sendDisconnWaitMessage() {
			removeMessages(MSG_DISCONN_WAIT);
			sendEmptyMessageDelayed(MSG_DISCONN_WAIT, DISCONN_WAIT_TIME);
		}

		@Override
		public void handleMessage(Message msg) {
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

			case MSG_DISCONN_WAIT:
				CavanAndroid.dLog("MSG_DISCONN_WAIT");
				onDisconnectWait();
				break;

			case MSG_CONNECT_TIMEOUT:
				CavanAndroid.dLog("MSG_CONNECT_TIMEOUT");
				if (!isGattConnected()) {
					onConnectTimeout();
				}
				break;

			case MSG_DISCOVER_TIMEOUT:
				CavanAndroid.dLog("MSG_DISCOVER_TIMEOUT");
				onDiscoverTimeout();
				break;
			}
		}
	}
}
