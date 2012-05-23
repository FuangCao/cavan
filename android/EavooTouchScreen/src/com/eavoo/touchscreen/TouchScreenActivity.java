package com.eavoo.touchscreen;

import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
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
	private FirmwarePathPreference mPreferenceUpgrade;

	private TouchScreen mTouchScreen;
	private TouchScreenCalibration mTouchScreenCalibration;

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		addPreferencesFromResource(R.xml.touchscreen);

		mPreferenceRoot = getPreferenceScreen();
		mPreferenceTouchScreenName = (PreferenceScreen) mPreferenceRoot.findPreference(KEY_TOUCHSCREEN_NAME);
		mPreferenceCalibration = (PreferenceScreen) mPreferenceRoot.findPreference(KEY_CALIBRATION);
		mPreferenceUpgrade = (FirmwarePathPreference) mPreferenceRoot.findPreference(KEY_UPGRADE_FIRMWIRE);

		mTouchScreen = new TouchScreen(this);
		mTouchScreenCalibration = new TouchScreenCalibration(this, mTouchScreen, mPreferenceCalibration);
	}

	@Override
	protected void onPause()
	{
		mTouchScreen.closeDevice();
		super.onPause();
	}

	@Override
	protected void onResume()
	{
		mTouchScreen.openDevice();
		mPreferenceTouchScreenName.setSummary(mTouchScreen.getDeviceName());
		super.onResume();
	}

	@Override
	public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference)
	{
		Log.i(TAG, "onPreferenceTreeClick = " + preference.getKey());

		if (preference == mPreferenceCalibration)
		{
			if (mTouchScreenCalibration.getPendding() == false)
			{
				mTouchScreenCalibration.start();
			}
		}
		else if (preference == mPreferenceUpgrade)
		{
		}

		return super.onPreferenceTreeClick(preferenceScreen, preference);
	}
}