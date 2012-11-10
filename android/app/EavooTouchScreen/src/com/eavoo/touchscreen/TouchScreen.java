package com.eavoo.touchscreen;

import java.io.File;

import android.content.Context;
import android.util.Log;

public class TouchScreen
{
	public static final String ACTION_CALIBRATION_COUNT_DOWN = "com.eavoo.touchscreen.calibration.count_down";
	public static final String ACTION_CALIBRATION_RUNNING = "com.eavoo.touchscreen.calibration.running";
	public static final String ACTION_CALIBRATION_SUCCESS = "com.eavoo.touchscreen.calibration.success";
	public static final String ACTION_CALIBRATION_FAILED = "com.eavoo.touchscreen.calibration.failed";

	public static final String ACTION_UPGRADE_RUNNING = "com.eavoo.touchscreen.upgrade.running";
	public static final String ACTION_UPGRADE_UNSUPPORT = "com.eavoo.touchscreen.upgrade.unsopport";
	public static final String ACTION_UPGRADE_SUCCESS = "com.eavoo.touchscreen.upgrade.success";
	public static final String ACTION_UPGRADE_FAILED = "com.eavoo.touchscreen.upgrade.failed";
	public static final String ACTION_UPGRADE_PATH_CHANGED = "com.eavoo.touchscreen.upgrade.path_changed";

	public static final String DEVICE_NAME_IT7260 = "IT7260";
	public static final String DEVICE_NAME_FT5406 = "FT5406";
	public static final String DEVICE_NAME_MT8C = "ChaCha_MT8C";
	public static final String DEVICE_NAME_ATMEGE168 = "ATMEGA168";
	public static final String DEVICE_NAME_GT800 = "GT800";

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
