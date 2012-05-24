package com.eavoo.touchscreen;

import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class FirmwareUpgradeThread extends Thread
{
	private static final String TAG = "FirmwareUpgradeThread";

	private Context mContext;
	private TouchScreen mTouchScreen;
	private String mFirmwarePath;

	@Override
	public void run()
	{
		Intent intent;

		intent = new Intent(TouchScreen.ACTION_UPGRADE_RUNNING);
		mContext.sendBroadcast(intent);

		Log.i(TAG, "Start upgrade firmware");

		boolean result;

		if (mTouchScreen.getDeviceName().equals(TouchScreen.DEVICE_NAME_FT5406))
		{
			result = mTouchScreen.UpgradeFt5406Native(mFirmwarePath);
		}
		else
		{
			intent = new Intent(TouchScreen.ACTION_UPGRADE_UNSUPPORT);
			mContext.sendBroadcast(intent);
			return;
		}

		if (result)
		{
			Log.i(TAG, "Upgrade firmware success");
			intent = new Intent(TouchScreen.ACTION_UPGRADE_SUCCESS);
		}
		else
		{
			intent = new Intent(TouchScreen.ACTION_UPGRADE_FAILED);
			Log.e(TAG, "Upgrade firmware failed");
		}

		mContext.sendBroadcast(intent);
	}

	public FirmwareUpgradeThread(TouchScreen touchscreen, String pathname)
	{
		super();

		this.mTouchScreen = touchscreen;
		this.mFirmwarePath = pathname;
		this.mContext = touchscreen.getContext();
	}
}
