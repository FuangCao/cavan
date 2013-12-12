package com.cavan.huahardwareinfo;

import java.io.File;

public class HuaGsensorDevice {
	private static final HuaGsensorDevice[] mGsensorDeviceList = {
		new HuaGsensorDevice("ADXL346", 0, 3),
		new HuaGsensorDevice("LIS3DH", 0, 3),
		new HuaGsensorDevice("LIS3DE", 0, 3),
		new HuaGsensorDevice("KXCJK", 0, 3),
		new HuaGsensorDevice("KXTIK", 0, 3),
		new HuaGsensorDevice("MMA845X", 0, 3),
		new HuaGsensorDevice("MXC6225XC", 0, 2),
		new HuaGsensorDevice("MXC6225XU", 0, 2),
		new HuaGsensorDevice("STK8313", 0, 3)
	};

	private File mFileDevice;
	private String mIcName;
	private int mVendorName;
	private int mAxisCount;

	public HuaGsensorDevice(String icName, int vendorName, int axisCount) {
		super();
		mIcName = icName;
		mFileDevice = new File("/dev", "HUA-" + icName);
		mVendorName = vendorName;
		mAxisCount = axisCount;
	}

	public File getFileDevice() {
		return mFileDevice;
	}

	public String getIcName() {
		return mIcName;
	}

	public int getVendorName() {
		return mVendorName;
	}

	public int getAxisCount() {
		return mAxisCount;
	}

	public static HuaGsensorDevice getGsensorDevice() {
		for (HuaGsensorDevice device : mGsensorDeviceList) {
			if (device.getFileDevice().exists()) {
				return device;
			}
		}

		return null;
	}
}