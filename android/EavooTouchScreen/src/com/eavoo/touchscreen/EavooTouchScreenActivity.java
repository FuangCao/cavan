package com.eavoo.touchscreen;

import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.util.Log;

public class EavooTouchScreenActivity extends PreferenceActivity
{
	private static final String TAG = "EavooTPActivity";

	private static final String KEY_CALIBRATION = "calibration";
	// private static final String KEY_UPGRADE_FT5406 = "upgrade_ft5406";
	private static final String KEY_TOUCHSCREEN_NAME = "touchscreen_name";

	private EavooTouchScreen mTouchScreen = new EavooTouchScreen();

	private PreferenceScreen mPreferenceScreen;
	private PreferenceScreen mCalibration;
	private PreferenceScreen mTouchScreenName;

	private String getTouchScreenDeviceName()
	{
		String name;

		name = mTouchScreen.getDeviceName();
		if (name == null)
		{
			return new String("UNKNOWN");
		}

		return name;
	}

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		addPreferencesFromResource(R.xml.touchscreen);

		mPreferenceScreen = getPreferenceScreen();
		mTouchScreenName = (PreferenceScreen) mPreferenceScreen.findPreference(KEY_TOUCHSCREEN_NAME);
		mCalibration = (PreferenceScreen) mPreferenceScreen.findPreference(KEY_CALIBRATION);

		mTouchScreenName.setSummary(getTouchScreenDeviceName());
	}

	@Override
	public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference)
	{
		if (preference == mCalibration)
		{
			Log.i(TAG, "Calibration");
			mTouchScreen.Calibration();
		}
		else if (preference == mTouchScreenName)
		{
			Log.i(TAG, "Touch Screen Name");
			mTouchScreenName.setSummary(getTouchScreenDeviceName());
		}

		return super.onPreferenceTreeClick(preferenceScreen, preference);
	}
}