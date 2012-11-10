package com.eavoo.touchscreen;

import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class TouchScreenCalibrationThread extends Thread
{
	private static final String TAG = "TouchScreenCalibration";

	private final TouchScreen mTouchScreen;
	private Context mContext;

	public TouchScreenCalibrationThread(TouchScreen touchscreen)
	{
		this.mTouchScreen = touchscreen;
		this.mContext = touchscreen.getContext();
	}

	@Override
	public void run()
	{
		Intent intent;

		for (int i = 5; i > 0; i--)
		{
			intent = new Intent(TouchScreen.ACTION_CALIBRATION_COUNT_DOWN);
			intent.putExtra("count", i);
			mContext.sendBroadcast(intent);

			try
			{
				sleep(1000);
			}
			catch (InterruptedException e)
			{
				e.printStackTrace();
			}
		}

		Log.i(TAG, "start calibration");

		intent = new Intent(TouchScreen.ACTION_CALIBRATION_RUNNING);
		mContext.sendBroadcast(intent);

		if (mTouchScreen.CalibrationNative())
		{
			intent = new Intent(TouchScreen.ACTION_CALIBRATION_SUCCESS);
			mContext.sendBroadcast(intent);
			Log.i(TAG, "calibration success");
		}
		else
		{
			intent = new Intent(TouchScreen.ACTION_CALIBRATION_FAILED);
			mContext.sendBroadcast(intent);
			Log.i(TAG, "calibration failed");
		}
	}
}
