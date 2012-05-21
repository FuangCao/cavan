package com.eavoo.touchscreen;

import java.io.File;

import android.util.Log;

public class EavooTouchScreen
{
	static private final String TAG = "EavooTouchScreen";
	static private final String TP_PROC_DEVICE = "/proc/swan_touchscreen";
	static private final String TP_MISC_DEVICE = "/dev/swan_touchscreen";

	private int mDeviceFd = -1;
	private String mDevicePath;
	private String mDeviceName;

	static
	{
		System.loadLibrary("EavooTouchScreen");
	}

	@Override
	protected void finalize() throws Throwable
	{
		CloseTouchScreenDeviceNative();
		super.finalize();
	}

	public EavooTouchScreen(String devpath)
	{
		for (String path : new String[] {devpath, TP_MISC_DEVICE, TP_PROC_DEVICE})
		{
			if (path == null)
			{
				continue;
			}

			if (setDevicePath(path))
			{
				break;
			}
		}
	}

	public EavooTouchScreen()
	{
		this(null);
	}

	public String getDevicePath()
	{
		return mDevicePath;
	}

	public boolean setDevicePath(String devpath)
	{
		File file = new File(devpath);
		if (file.exists() == false)
		{
			return false;
		}

		if (OpenTouchscreenDeviceNative(file.getAbsolutePath()) == false)
		{
			return false;
		}

		Log.i(TAG, "mDevicePath = " + mDevicePath);
		Log.i(TAG, "mDeviceName = " + mDeviceName);

		return true;
	}

	public String getDeviceName()
	{
		return mDeviceName;
	}

	native private boolean OpenTouchscreenDeviceNative(String devpath);
	native private boolean CloseTouchScreenDeviceNative();

	native public boolean CalibrationNative();
	native public boolean UpgradeFt5406Native(String filename);
}
