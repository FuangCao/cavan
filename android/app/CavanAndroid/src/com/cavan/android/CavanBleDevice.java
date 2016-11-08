package com.cavan.android;

import android.bluetooth.BluetoothDevice;

public class CavanBleDevice implements Comparable<CavanBleDevice> {

	private int mRssi;
	private int mIndex;
	private byte[] mScanRecord;
	private BluetoothDevice mDevice;

	public CavanBleDevice(BluetoothDevice device) {
		super();
		mIndex = -1;
		mDevice = device;
	}

	public void setIndex(int index) {
		mIndex = index;
	}

	public int getIndex() {
		return mIndex;
	}

	public void setRssi(int rssi) {
		mRssi = rssi;
	}

	public int getRssi() {
		return mRssi;
	}

	public String getName() {
		return mDevice.getName();
	}

	public BluetoothDevice getDevice() {
		return mDevice;
	}

	public byte[] getScanRecord() {
		return mScanRecord;
	}

	public void setScanRecord(byte[] scanRecord) {
		mScanRecord = scanRecord;
	}

	@Override
	public String toString() {
		String name = mDevice.getName();
		if (name != null && name.length() > 0) {
			return String.format("%s - %s: %d", mDevice.getAddress(), name, mRssi);
		}

		return String.format("%s: %d", mDevice.getAddress(), mRssi);
	}

	@Override
	public int compareTo(CavanBleDevice another) {
		return another.getRssi() - mRssi;
	}
}
