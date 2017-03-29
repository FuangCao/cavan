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

	private static final int START_SCAN_OVERTIME = 3000;

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

	protected void onScanResult(CavanBleDevice[] devices, CavanBleDevice device) {}
	protected void onAutoSelected(CavanBleDevice device) {}

	private Handler mHandler = new Handler() {

		@SuppressWarnings("deprecation")
		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_AUTO_SELECT:
				CavanAndroid.dLog("MSG_AUTO_SELECT: enable = " + mScanEnable);
				mHandler.removeMessages(MSG_AUTO_SELECT);
				if (mScanEnable && mAutoSelectDelay > 0 && mAddresses == null) {
					onAutoSelected(mDeviceBest);
				}
				break;

			case MSG_START_SCAN:
				CavanAndroid.dLog("MSG_START_SCAN: enable = " + mScanEnable);
				mHandler.removeMessages(MSG_START_SCAN);

				if (mScanEnable) {
					mHandler.sendEmptyMessageDelayed(MSG_START_SCAN, START_SCAN_OVERTIME);
					mAdapter.stopLeScan(CavanBleScanner.this);
					mAdapter.startLeScan(mUuids, CavanBleScanner.this);
				}
				break;

			case MSG_SCAN_RESULT:
				CavanBleDevice device = (CavanBleDevice) msg.obj;

				synchronized (mDeviceMap) {
					Collection<CavanBleDevice> values = mDeviceMap.values();
					CavanBleDevice[] devices = new CavanBleDevice[values.size()];
					values.toArray(devices);
					Arrays.sort(devices);

					mDeviceBest = devices[0];
					onScanResult(devices, device);
				}

				if (mAddresses != null) {
					if (mScanEnable && isAddressMatchedInternal(device.getAddress())) {
						onAutoSelected(device);
					}
				} else if (mAutoSelectDelay > 0 && mHandler.hasMessages(MSG_AUTO_SELECT) == false) {
					mHandler.sendEmptyMessageDelayed(MSG_AUTO_SELECT, mAutoSelectDelay);
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
		mAutoSelectDelay = delay;
		CavanAndroid.dLog("mAutoSelectDelay = " + mAutoSelectDelay);
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

	@SuppressWarnings("deprecation")
	public void stopScan() {
		mScanEnable = false;
		mAdapter.stopLeScan(this);

		cleaup();
	}

	@Override
	protected void finalize() throws Throwable {
		stopScan();
	}

	@Override
	public void onLeScan(BluetoothDevice btDevice, int rssi, byte[] scanRecord) {
		mHandler.removeMessages(MSG_START_SCAN);

		if (mNames.size() > 0) {
			String name = btDevice.getName();
			if (name == null || mNames.indexOf(name) < 0) {
				return;
			}
		}

		synchronized (mDeviceMap) {
			String address = btDevice.getAddress();
			CavanBleDevice device = mDeviceMap.get(address);
			if (device == null) {
				device = new CavanBleDevice(btDevice);
				mDeviceMap.put(address, device);
			}

			device.setRssi(rssi);
			device.setScanRecord(scanRecord);

			if (!mHandler.hasMessages(MSG_SCAN_RESULT, device)) {
				Message message = mHandler.obtainMessage(MSG_SCAN_RESULT, device);
				mHandler.sendMessageDelayed(message, 500);
			}
		}
	}

	@Override
	protected void onBluetoothAdapterStateChanged(boolean enabled) {
		if (enabled) {
			mHandler.sendEmptyMessage(MSG_START_SCAN);
		}
	}
}