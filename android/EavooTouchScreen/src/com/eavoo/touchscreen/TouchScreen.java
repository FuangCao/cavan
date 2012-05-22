package com.eavoo.touchscreen;

import java.io.File;

import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.util.Log;

public class TouchScreen
{
	public static final String ACTION_CALIBRATION_SUMMARY_INT = "com.eavoo.touchscreen.calibration.summary_int";
	public static final String ACTION_CALIBRATION_SUMMARY_STRING = "com.eavoo.touchscreen.calibration.summary_string";
	public static final String ACTION_CALIBRATION_RUNNING = "com.eavoo.touchscreen.calibration.running";
	public static final String ACTION_CALIBRATION_COMPLETE = "com.eavoo.touchscreen.calibration.complete";

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

	public TouchScreen(Context context, String devpath)
	{
		this.mContext = context;

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

	public TouchScreen(Context context)
	{
		this(context, null);
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

	public Context getContext()
	{
		return mContext;
	}

	public Resources getResources()
	{
		return mContext.getResources();
	}

	public void sendBroadcast(String action, String value)
	{
		Intent intent = new Intent(action);
		intent.putExtra("content", value);
		mContext.sendBroadcast(intent);
	}

	public void sendBroadcast(String action, int value)
	{
		Intent intent = new Intent(action);
		intent.putExtra("content", value);
		mContext.sendBroadcast(intent);
	}

	public int getExtra(Intent intent, int defaultValue)
	{
		return intent.getIntExtra("content", defaultValue);
	}

	public String getExtra(Intent intent)
	{
		return intent.getStringExtra("content");
	}

	native private boolean OpenTouchscreenDeviceNative(String devpath);
	native private boolean CloseTouchScreenDeviceNative();

	native public boolean CalibrationNative();
	native public boolean UpgradeFt5406Native(String filename);
}
