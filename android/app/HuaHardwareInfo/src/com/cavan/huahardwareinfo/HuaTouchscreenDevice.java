package com.cavan.huahardwareinfo;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

import android.R.integer;
import android.os.Build;

public class HuaTouchscreenDevice {
	private static final HuaTouchscreenDevice[] mTouchscreenList = {
		new HuaTouchscreenDevice("CY8C242", "/sys/bus/i2c/devices/i2c-2/2-0024/firmware_id", "cy8c242.iic"),
		new HuaTouchscreenDevice("FT6306", "/dev/FT5216", "/sys/bus/i2c/devices/i2c-2/2-0038/firmware_id", "FT6306.bin")
	};

	private String mIcName;
	private String mFwName;
	private File mFileDevice;
	private File mFileFwId;
	private int mFwId;
	private HuaTouchScreenVendorInfo mVendorInfo;

	public HuaTouchscreenDevice(String icName, String devPath, String fwIdPath, String fwName) {
		super();
		mIcName = icName;
		mFwName = fwName;
		mFileDevice = new File(devPath);
		mFileFwId = new File(fwIdPath);
	}

	public HuaTouchscreenDevice(String icName, String fwIdPath, String fwName) {
		this(icName, "/dev/HUA-" + icName, fwIdPath, fwName);
	}

	private String readFwId() {
		FileInputStream inputStream;
		try {
			inputStream = new FileInputStream(mFileFwId);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			return null;
		}

		byte[] buff = new byte[4];

		try {
			int length = inputStream.read(buff);
			return new String(buff);
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			try {
				inputStream.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		return null;
	}

	private void fillVendorInfo() {
		String fwIdContent = readFwId();

		if (fwIdContent == null) {
			mFwId = 0;
		} else {
			mFwId = Integer.parseInt(fwIdContent, 16);
		}

		mVendorInfo = new HuaTouchScreenVendorInfo(mFwId);
	}

	public static HuaTouchscreenDevice getTouchscreenDevice() {
		for (HuaTouchscreenDevice device : mTouchscreenList) {
			if (device.mFileDevice.exists()) {
				device.fillVendorInfo();
				return device;
			}
		}

		return null;
	}

	public String getIcName() {
		return mIcName;
	}

	public File getFileDevice() {
		return mFileDevice;
	}

	public File getFileFwId() {
		return mFileFwId;
	}

	public int getFwId() {
		return mFwId;
	}

	public String getFwName() {
		if (mVendorInfo == null) {
			return null;
		}

		return Build.BOARD + "_" + mFwName + "_" + mVendorInfo.getShortName();
	}

	public HuaTouchScreenVendorInfo getVendorInfo() {
		return mVendorInfo;
	}
}