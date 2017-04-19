package com.cavan.android;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.UUID;

import android.bluetooth.BluetoothAdapter.LeScanCallback;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.os.Handler;
import android.os.Message;

public class CavanBleScanner extends CavanBluetoothAdapter implements LeScanCallback {

	private static final int START_SCAN_OVERTIME = 5000;

	private static final int MSG_AUTO_SELECT = 1;
	private static final int MSG_START_SCAN = 2;
	private static final int MSG_SCAN_RESULT = 3;

	private UUID[] mUuids;
	private String[] mAddresses;
	private boolean mScanEnable;
	private long mAutoSelectDelay;
	private CavanBleDevice mDeviceBest;
	private ArrayList<String> mNames = new ArrayList<String>();
	private HashMap<String, CavanBleDevice> mDeviceMap = new HashMap<String, CavanBleDevice>();

	protected void onScanStarted() {
		CavanAndroid.dLog("onScanStarted");
	}

	protected void onScanResult(CavanBleDevice[] devices, CavanBleDevice device) {
		CavanAndroid.dLog("onScanResult: " + device);
	}

	protected void onAutoSelected(CavanBleDevice device) {
		CavanAndroid.dLog("onAutoSelected: " + device);
	}

	protected void onBestDeviceChanged(CavanBleDevice device) {
		CavanAndroid.dLog("onBestDeviceChanged: " + device);
	}

	private Handler mHandler = new Handler() {

		@SuppressWarnings("deprecation")
		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_AUTO_SELECT:
				CavanAndroid.dLog("MSG_AUTO_SELECT: enable = " + mScanEnable);
				mHandler.removeMessages(MSG_AUTO_SELECT);

				if (mScanEnable && mAutoSelectDelay > 0) {
					CavanBleDevice device = mDeviceBest;

					if (device == null) {
						sendEmptyMessageDelayed(MSG_AUTO_SELECT, mAutoSelectDelay);
					} else {
						onAutoSelected(device);
					}
				}
				break;

			case MSG_START_SCAN:
				CavanAndroid.dLog("MSG_START_SCAN: enable = " + mScanEnable);
				mHandler.removeMessages(MSG_START_SCAN);

				if (mScanEnable) {
					mHandler.sendEmptyMessageDelayed(MSG_START_SCAN, START_SCAN_OVERTIME);
					mAdapter.stopLeScan(CavanBleScanner.this);
					mAdapter.startLeScan(mUuids, CavanBleScanner.this);
					onScanStarted();
				}
				break;

			case MSG_SCAN_RESULT:
				CavanBleDevice[] devices;
				CavanBleDevice device = (CavanBleDevice) msg.obj;

				synchronized (mDeviceMap) {
					Collection<CavanBleDevice> values = mDeviceMap.values();

					devices = new CavanBleDevice[values.size()];
					values.toArray(devices);
					Arrays.sort(devices);

					onScanResult(devices, device);
				}

				if (mAddresses != null) {
					if (isAddressMatchedInternal(device.getAddress())) {
						setBestDevice(device);
					}
				} else {
					setBestDevice(devices[0]);
				}
				break;
			}
		}
	};

	public CavanBleScanner(Context context) {
		super(context);
	}

	private boolean isAddressMatchedInternal(String address) {
		for (String node : mAddresses) {
			if (node.equalsIgnoreCase(address)) {
				return true;
			}
		}

		return false;
	}

	public boolean isAddressMatched(String address) {
		if (mAddresses == null) {
			return false;
		}

		return isAddressMatchedInternal(address);
	}

	public void addName(String name) {
		if (mNames.indexOf(name) < 0) {
			mNames.add(name);
		}
	}

	public void setAutoSelect(long delay) {
		setBestDevice(null);
		mAutoSelectDelay = delay;
		CavanAndroid.dLog("mAutoSelectDelay = " + mAutoSelectDelay);
	}

	public void setBestDevice(CavanBleDevice device) {
		if (mDeviceBest != device) {
			mDeviceBest = device;

			if (device != null) {
				onBestDeviceChanged(device);

				if (mAutoSelectDelay > 0 && !mHandler.hasMessages(MSG_AUTO_SELECT)) {
					mHandler.sendEmptyMessageDelayed(MSG_AUTO_SELECT, mAutoSelectDelay);
				}
			}
		}
	}

	public void startScan(UUID[] uuids, String[] names, String[] addresses) {
		mScanEnable = true;
		mUuids = uuids;

		if (addresses != null && addresses.length > 0) {
			mAddresses = addresses;

			for (int i = 0; i < addresses.length; i++) {
				CavanAndroid.dLog("address" + i + ". " + addresses[i]);
			}
		} else {
			mAddresses = null;
			CavanAndroid.dLog("mAddresses is empty");
		}

		if (names != null) {
			for (String name : names) {
				addName(name);
			}
		}

		mHandler.sendEmptyMessage(MSG_START_SCAN);
	}

	public void startScan(UUID[] uuids, String[] names) {
		startScan(uuids, names, null);
	}

	public void startScan(UUID[] uuids) {
		startScan(uuids, null, null);
	}

	public void startScan(String[] names) {
		startScan(null, names, null);
	}

	public void startScan() {
		startScan(null, null, null);
	}

	public void post(Runnable runnable, long delayMillis) {
		mHandler.postDelayed(runnable, delayMillis);
	}

	public CavanBleDevice putBluetoothDevice(BluetoothDevice device, int rssi, byte[] scanRecord) {
		CavanBleDevice myDevice;

		synchronized (mDeviceMap) {
			String address = device.getAddress();
			myDevice = mDeviceMap.get(address);
			if (myDevice == null) {
				myDevice = new CavanBleDevice(device);
				mDeviceMap.put(address, myDevice);
			}

			myDevice.setRssi(rssi);
			myDevice.setScanRecord(scanRecord);

			if (!mHandler.hasMessages(MSG_SCAN_RESULT, myDevice)) {
				Message message = mHandler.obtainMessage(MSG_SCAN_RESULT, myDevice);
				mHandler.sendMessageDelayed(message, 500);
			}
		}

		return myDevice;
	}

	public CavanBleDevice addBluetoothDevice(BluetoothDevice device) {
		return putBluetoothDevice(device, 0, null);
	}

	public CavanBleDevice addBluetoothDevice(String address) {
		BluetoothDevice device = getRemoteDevice(address);
		if (device == null) {
			return null;
		}

		return addBluetoothDevice(device);
	}

	@SuppressWarnings("deprecation")
	public void stopScan() {
		mScanEnable = false;
		mAdapter.stopLeScan(this);
	}

	@Override
	protected void finalize() throws Throwable {
		stopScan();
	}

	@Override
	public void onLeScan(BluetoothDevice device, int rssi, byte[] scanRecord) {
		mHandler.removeMessages(MSG_START_SCAN);

		if (mNames.size() > 0) {
			String name = device.getName();
			if (name == null || mNames.indexOf(name) < 0) {
				return;
			}
		}

		putBluetoothDevice(device, rssi, scanRecord);
	}

	@Override
	protected void onBluetoothAdapterStateChanged(boolean enabled) {
		if (enabled) {
			mHandler.sendEmptyMessage(MSG_START_SCAN);
		}
	}
}
