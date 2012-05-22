package com.eavoo.touchscreen;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.MutableContextWrapper;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.text.method.Touch;
import android.util.Log;

public class TouchScreenActivity extends PreferenceActivity
{
	private static final String TAG = "TouchScreenActivity";

	private static final String KEY_CALIBRATION = "calibration";
	private static final String KEY_UPGRADE_FIRMWIRE = "upgrade_firmwire";
	private static final String KEY_TOUCHSCREEN_NAME = "touchscreen_name";

	private PreferenceScreen mPreferenceRoot;
	private PreferenceScreen mPreferenceCalibration;
	private PreferenceScreen mPreferenceTouchScreenName;
	private PreferenceScreen mPreferenceUpgrade;

	private TouchScreen mTouchScreen;
	private boolean mCalibrationPedding = false;

	private IntentFilter mIntentFilter;
	private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver()
	{
		@Override
		public void onReceive(Context context, Intent intent)
		{
			String action = intent.getAction();

			if (action.equalsIgnoreCase(TouchScreen.ACTION_CALIBRATION_COMPLETE))
			{
				mCalibrationPedding = false;
			}
			else if (action.equalsIgnoreCase(TouchScreen.ACTION_CALIBRATION_RUNNING))
			{
				mCalibrationPedding = true;
			}
		}
	};

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		addPreferencesFromResource(R.xml.touchscreen);

		mPreferenceRoot = getPreferenceScreen();
		mPreferenceTouchScreenName = (PreferenceScreen) mPreferenceRoot.findPreference(KEY_TOUCHSCREEN_NAME);
		mPreferenceCalibration = (PreferenceScreen) mPreferenceRoot.findPreference(KEY_CALIBRATION);
		mPreferenceUpgrade = (PreferenceScreen) mPreferenceRoot.findPreference(KEY_UPGRADE_FIRMWIRE);

		mTouchScreen = new TouchScreen(this);
		mPreferenceTouchScreenName.setSummary(mTouchScreen.getDeviceName());

		mIntentFilter = new IntentFilter(TouchScreen.ACTION_CALIBRATION_RUNNING);
		mIntentFilter.addAction(TouchScreen.ACTION_CALIBRATION_COMPLETE);
		registerReceiver(mBroadcastReceiver, mIntentFilter);
	}

	@Override
	public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference)
	{
		if (preference == mPreferenceCalibration)
		{
			if (mCalibrationPedding == false)
			{
				mCalibrationPedding = true;
				TouchScreenCalibration calibration = new TouchScreenCalibration(this, mTouchScreen, mPreferenceCalibration);
				calibration.start();
			}
		}
		else if (preference == mPreferenceUpgrade)
		{
		}

		return super.onPreferenceTreeClick(preferenceScreen, preference);
	}
}