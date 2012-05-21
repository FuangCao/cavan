package com.eavoo.touchscreen;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceScreen;
import android.util.Log;

public class EavooTouchScreenCalibration extends Thread implements OnPreferenceClickListener
{
	private static final String TAG = "EavooTouchScreenCalibration";
	private static final String ACTION_SUMMARY_CHANGED = "com.eavoo.touchscreen.calibration.summary_changed";

	private final Context mContext;
	private final PreferenceScreen mPreferenceScreen;
	private final EavooTouchScreen mTouchScreen;
	
	private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver()
	{
		@Override
		public void onReceive(Context context, Intent intent)
		{
			String action = intent.getAction();

			Log.i(TAG, "action = " + action);

			if (action.equalsIgnoreCase(ACTION_SUMMARY_CHANGED))
			{
				mPreferenceScreen.setSummary(intent.getIntExtra("summary", 0));
			}
		}
	};

	private final IntentFilter mIntentFilter;

	@Override
	public void run()
	{
		Intent intent;
		
		intent = new Intent(ACTION_SUMMARY_CHANGED);
		intent.putExtra("summary", R.string.calibration_summary_running);
		mContext.sendBroadcast(intent);

		Log.i(TAG, "start calibration");

		if (mTouchScreen.CalibrationNative())
		{
			intent = new Intent(ACTION_SUMMARY_CHANGED);
			intent.putExtra("summary", R.string.calibration_summary_success);
			mContext.sendBroadcast(intent);
			Log.e(TAG, "calibration success");
		}
		else
		{
			intent = new Intent(ACTION_SUMMARY_CHANGED);
			intent.putExtra("summary", R.string.calibration_summary_failed);
			mContext.sendBroadcast(intent);
			Log.i(TAG, "calibration failed");
		}
	}

	public EavooTouchScreenCalibration(Context context, EavooTouchScreen touchscreen, PreferenceScreen preference)
	{
		this.mContext = context;
		this.mPreferenceScreen = preference;
		this.mTouchScreen = touchscreen;

		mIntentFilter = new IntentFilter(ACTION_SUMMARY_CHANGED);
		mContext.registerReceiver(mBroadcastReceiver, mIntentFilter);
		mPreferenceScreen.setPersistent(false);
	}

	@Override
	public boolean onPreferenceClick(Preference arg0)
	{
		start();
		
		return true;
	}
}
