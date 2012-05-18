package com.eavoo.touchscreen;

import java.io.File;

import android.util.Log;

public class EavooTouchScreen
{
	static private final String TAG = "EavooTP";
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
		CloseTouchScreenDevice();
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

		if (OpenTouchscreenDevice(file.getAbsolutePath()) == false)
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

	native private boolean OpenTouchscreenDevice(String devpath);
	native private boolean CloseTouchScreenDevice();

	native public boolean Calibration();
	native public boolean UpgradeFt5406(String filename);
}
