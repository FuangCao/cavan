package com.eavoo.touchscreen;

import java.io.File;

import android.content.Context;
import android.content.res.Resources;
import android.util.Log;

public class TouchScreen
{
	private static final String TAG = "TouchScreen";
	private static final String TP_PROC_DEVICE = "/proc/swan_touchscreen";
	private static final String TP_MISC_DEVICE = "/dev/swan_touchscreen";

	private int mDeviceFd = -1;
	private String mDevicePath;
	private String mDeviceName;
	private Context mContext;

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

	public TouchScreen(Context context)
	{
		this.mContext = context;
	}

	public String getDevicePath()
	{
		return mDevicePath;
	}

	public boolean setDevicePath(String devpath)
	{
		File file = new File(devpath);
		if (file.canRead() == false)
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

	public Context getContext()
	{
		return mContext;
	}

	public Resources getResources()
	{
		return mContext.getResources();
	}

	public boolean closeDevice()
	{
		return CloseTouchScreenDeviceNative();
	}

	public boolean openDevice()
	{
		for (String path : new String[] {TP_MISC_DEVICE, TP_PROC_DEVICE})
		{
			if (setDevicePath(path))
			{
				return true;
			}
		}

		return false;
	}

	native private boolean OpenTouchscreenDeviceNative(String devpath);
	native private boolean CloseTouchScreenDeviceNative();

	native public boolean CalibrationNative();
	native public boolean UpgradeFt5406Native(String filename);
}
