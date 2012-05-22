package com.eavoo.touchscreen;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceScreen;
import android.util.Log;

public class TouchScreenCalibration extends Thread
{
	private static final String TAG = "TouchScreenCalibration";

	private final PreferenceScreen mPreferenceScreen;
	private final TouchScreen mTouchScreen;

	private final IntentFilter mIntentFilter;
	private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver()
	{
		@Override
		public void onReceive(Context context, Intent intent)
		{
			String action = intent.getAction();

			if (action.equalsIgnoreCase(TouchScreen.ACTION_CALIBRATION_SUMMARY_INT))
			{
				mPreferenceScreen.setSummary(mTouchScreen.getExtra(intent, R.string.calibration_summary));
			}
			else if (action.equalsIgnoreCase(TouchScreen.ACTION_CALIBRATION_SUMMARY_STRING))
			{
				mPreferenceScreen.setSummary(mTouchScreen.getExtra(intent));
			}
		}
	};

	private void setPreferenceSummary(int value)
	{
		mTouchScreen.sendBroadcast(TouchScreen.ACTION_CALIBRATION_SUMMARY_INT, value);
	}

	private void setPreferenceSummary(String value)
	{
		mTouchScreen.sendBroadcast(TouchScreen.ACTION_CALIBRATION_SUMMARY_STRING, value);
	}

	@Override
	public void run()
	{
		mTouchScreen.sendBroadcast(TouchScreen.ACTION_CALIBRATION_RUNNING, 0);

		for (int i = 5; i > 0; i--)
		{
			setPreferenceSummary(mTouchScreen.getResources().getString(R.string.calibration_summary_start, i));

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

		setPreferenceSummary(R.string.calibration_summary_running);

		if (mTouchScreen.CalibrationNative())
		{
			setPreferenceSummary(R.string.calibration_summary_success);
			Log.e(TAG, "calibration success");
			mTouchScreen.sendBroadcast(TouchScreen.ACTION_CALIBRATION_COMPLETE, 0);
		}
		else
		{
			setPreferenceSummary(R.string.calibration_summary_failed);
			Log.i(TAG, "calibration failed");
			mTouchScreen.sendBroadcast(TouchScreen.ACTION_CALIBRATION_COMPLETE, -1);
		}
	}

	public TouchScreenCalibration(Context context, TouchScreen touchscreen, PreferenceScreen preference)
	{
		this.mPreferenceScreen = preference;
		this.mTouchScreen = touchscreen;

		mIntentFilter = new IntentFilter(TouchScreen.ACTION_CALIBRATION_SUMMARY_INT);
		mIntentFilter.addAction(TouchScreen.ACTION_CALIBRATION_SUMMARY_STRING);
		mTouchScreen.getContext().registerReceiver(mBroadcastReceiver, mIntentFilter);
		mPreferenceScreen.setPersistent(false);
	}

	@Override
	protected void finalize() throws Throwable
	{
		mTouchScreen.getContext().unregisterReceiver(mBroadcastReceiver);
	}
}
