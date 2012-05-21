package com.eavoo.touchscreen;

import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.util.Log;

public class EavooTouchScreenActivity extends PreferenceActivity
{
	private static final String TAG = "EavooTouchScreenActivity";

	private static final String KEY_CALIBRATION = "calibration";
	// private static final String KEY_UPGRADE_FT5406 = "upgrade_ft5406";
	private static final String KEY_TOUCHSCREEN_NAME = "touchscreen_name";

	private EavooTouchScreen mTouchScreen;

	private PreferenceScreen mPreferenceScreen;
	private PreferenceScreen mPreferenceCalibration;
	private PreferenceScreen mPreferenceTouchScreenName;

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		addPreferencesFromResource(R.xml.touchscreen);

		mPreferenceScreen = getPreferenceScreen();
		mPreferenceTouchScreenName = (PreferenceScreen) mPreferenceScreen.findPreference(KEY_TOUCHSCREEN_NAME);
		mPreferenceCalibration = (PreferenceScreen) mPreferenceScreen.findPreference(KEY_CALIBRATION);

		mTouchScreen = new EavooTouchScreen();
		mPreferenceTouchScreenName.setSummary(mTouchScreen.getDeviceName());
	}

	@Override
	public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference)
	{
		if (preference == mPreferenceCalibration)
		{
			EavooTouchScreenCalibration calibration = new EavooTouchScreenCalibration(this, mTouchScreen, mPreferenceCalibration);
			calibration.start();
		}

		return super.onPreferenceTreeClick(preferenceScreen, preference);
	}
}