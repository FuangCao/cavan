package com.cavan.android;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.Timer;
import java.util.TimerTask;
import java.util.UUID;

import android.bluetooth.BluetoothAdapter.LeScanCallback;
import android.bluetooth.BluetoothDevice;
import android.content.Context;

public class CavanBleScanner extends CavanBluetoothAdapter implements LeScanCallback {

	private UUID[] mUuids;
	private boolean mScanEnable;
	private long mAutoSelectDelay;
	private Timer mTimerAutoSelect;
	private TimerTask mTaskAutoSelect;
	private CavanBleDevice mDeviceBest;
	private ArrayList<String> mNames = new ArrayList<String>();
	private HashMap<String, CavanBleDevice> mDeviceMap = new HashMap<String, CavanBleDevice>();

	protected void onScanResult(CavanBleDevice[] devices, CavanBleDevice device) {}
	protected void onAutoSelected(CavanBleDevice device) {}

	public CavanBleScanner(Context context) {
		super(context);
	}

	@Override
	protected void finalize() throws Throwable {
		if (mTaskAutoSelect != null) {
			mTaskAutoSelect.cancel();
		}

		if (mTimerAutoSelect != null) {
			mTimerAutoSelect.cancel();
		}

		super.finalize();
	}

	public void addName(String name) {
		if (mNames.indexOf(name) < 0) {
			mNames.add(name);
		}
	}

	public void setAutoSelect(long delay) {
		mAutoSelectDelay = delay;

		if (delay <= 0 && mTaskAutoSelect != null) {
			mTaskAutoSelect.cancel();
			mTaskAutoSelect = null;
		}

		CavanAndroid.eLog("mAutoSelectDelay = " + mAutoSelectDelay);
	}

	@SuppressWarnings("deprecation")
	private boolean startScanInternal() {
		CavanAndroid.eLog("startLeScan");
		return mAdapter.startLeScan(mUuids, this);
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
		if (mTaskAutoSelect != null) {
			mTaskAutoSelect.cancel();
			mTaskAutoSelect = null;
		}

		cleaup();
	}

	@Override
	public void onLeScan(BluetoothDevice btDevice, int rssi, byte[] scanRecord) {
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

		if (mAutoSelectDelay > 0 && mTaskAutoSelect == null) {
			if (mTimerAutoSelect == null) {
				mTimerAutoSelect = new Timer("CavanBleScanner");
			}

			mTaskAutoSelect = new TimerTask() {

				@Override
				public void run() {
					onAutoSelected(mDeviceBest);
				}
			};

			mTimerAutoSelect.schedule(mTaskAutoSelect, mAutoSelectDelay);
		}
	}

	@Override
	protected void onBluetoothAdapterStateChanged(boolean enabled) {
		if (enabled && mScanEnable) {
			startScanInternal();
		}
	}
}