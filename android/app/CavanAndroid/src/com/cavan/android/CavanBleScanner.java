package com.cavan.android;

import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.UUID;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothAdapter.LeScanCallback;
import android.bluetooth.BluetoothDevice;

public class CavanBleScanner implements LeScanCallback {

	private String mName;
	private BluetoothAdapter mAdapter;
	private HashMap<String, CavanBleDevice> mDeviceMap = new HashMap<String, CavanBleDevice>();

	protected void onScanResult(CavanBleDevice[] devices, CavanBleDevice device) {}

	public CavanBleScanner(BluetoothAdapter adapter) {
		adapter.enable();
		mAdapter = adapter;
	}

	public void setName(String name) {
		mName = name;
	}

	@SuppressWarnings("deprecation")
	public boolean startScan(UUID[] uuids, String name) {
		mName = name;
		return mAdapter.startLeScan(uuids, this);
	}

	public boolean startScan(UUID[] uuids) {
		return startScan(uuids, null);
	}

	public boolean startScan(String name) {
		return startScan(null, name);
	}

	public boolean startScan() {
		return startScan(null, null);
	}

	@SuppressWarnings("deprecation")
	public void stopScan() {
		mAdapter.stopLeScan(this);
	}

	@Override
	public void onLeScan(BluetoothDevice device, int rssi, byte[] scanRecord) {
		if (mName != null) {
			String name = device.getName();
			if (name == null || name.equals(mName) == false) {
				return;
			}
		}

		String address = device.getAddress();
		CavanBleDevice bleDevice = mDeviceMap.get(address);
		if (bleDevice == null) {
			bleDevice = new CavanBleDevice(device);
			mDeviceMap.put(address, bleDevice);
		}

		bleDevice.setRssi(rssi);

		Collection<CavanBleDevice> values = mDeviceMap.values();
		CavanBleDevice[] devices = new CavanBleDevice[values.size()];
		values.toArray(devices);
		Arrays.sort(devices);
		onScanResult(devices, bleDevice);
	}
}