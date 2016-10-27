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

	private UUID[] mUuids;
	private boolean mScanEnable;
	private long mAutoSelectDelay;
	private CavanBleDevice mDeviceBest;
	private ArrayList<String> mNames = new ArrayList<String>();
	private HashMap<String, CavanBleDevice> mDeviceMap = new HashMap<String, CavanBleDevice>();

	protected void onScanResult(CavanBleDevice[] devices, CavanBleDevice device) {}
	protected void onAutoSelected(CavanBleDevice device) {}

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_AUTO_SELECT:
				CavanAndroid.dLog("MSG_AUTO_SELECT");
				mHandler.removeMessages(MSG_AUTO_SELECT);
				if (mScanEnable && mAutoSelectDelay > 0) {
					onAutoSelected(mDeviceBest);
				}
				break;

			case MSG_START_SCAN:
				CavanAndroid.dLog("MSG_START_SCAN");
				startScanInternal();
				break;
			}
		}
	};

	public CavanBleScanner(Context context) {
		super(context);
	}

	public void addName(String name) {
		if (mNames.indexOf(name) < 0) {
			mNames.add(name);
		}
	}

	public void setAutoSelect(long delay) {
		mAutoSelectDelay = delay;
		CavanAndroid.eLog("mAutoSelectDelay = " + mAutoSelectDelay);
	}

	@SuppressWarnings("deprecation")
	private boolean startScanInternal() {
		CavanAndroid.eLog("startLeScan: enable = " + mScanEnable);

		mHandler.removeMessages(MSG_START_SCAN);

		if (mScanEnable) {
			mHandler.sendEmptyMessageDelayed(MSG_START_SCAN, START_SCAN_OVERTIME);
			return mAdapter.startLeScan(mUuids, this);
		}

		return false;
	}

	public boolean startScan(UUID[] uuids, String[] names) {
		mScanEnable = true;
		mUuids = uuids;

		if (names != null) {
			for (String name : names) {
				addName(name);
			}
		}

		return startScanInternal();
	}

	public boolean startScan(UUID[] uuids) {
		return startScan(uuids, null);
	}

	public boolean startScan(String[] names) {
		return startScan(null, names);
	}

	public boolean startScan() {
		return startScan(null, null);
	}

	@SuppressWarnings("deprecation")
	public void stopScan() {
		mScanEnable = false;
		mAdapter.stopLeScan(this);

		cleaup();
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

		String address = btDevice.getAddress();
		CavanBleDevice device = mDeviceMap.get(address);
		if (device == null) {
			device = new CavanBleDevice(btDevice);
			mDeviceMap.put(address, device);
		}

		device.setRssi(rssi);

		Collection<CavanBleDevice> values = mDeviceMap.values();
		CavanBleDevice[] devices = new CavanBleDevice[values.size()];
		values.toArray(devices);
		Arrays.sort(devices);
		onScanResult(devices, device);

		mDeviceBest = devices[0];

		if (mAutoSelectDelay > 0 && mHandler.hasMessages(MSG_AUTO_SELECT) == false) {
			mHandler.sendEmptyMessageDelayed(MSG_AUTO_SELECT, mAutoSelectDelay);
		}
	}

	@Override
	protected void onBluetoothAdapterStateChanged(boolean enabled) {
		if (enabled) {
			startScanInternal();
		}
	}
}