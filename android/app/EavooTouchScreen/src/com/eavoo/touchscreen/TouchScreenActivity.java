package com.eavoo.touchscreen;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.util.Log;

public class TouchScreenActivity extends PreferenceActivity
{
	private static final String TAG = "TouchScreenActivity";

	private static final String KEY_CALIBRATION = "calibration";
	private static final String KEY_UPGRADE_FIRMWIRE = "upgrade_firmwire";
	private static final String KEY_TOUCHSCREEN_NAME = "touchscreen_name";

	private static final String[] mBroadcastActions =
	{
		TouchScreen.ACTION_CALIBRATION_COUNT_DOWN,
		TouchScreen.ACTION_CALIBRATION_FAILED,
		TouchScreen.ACTION_CALIBRATION_RUNNING,
		TouchScreen.ACTION_CALIBRATION_SUCCESS,
		TouchScreen.ACTION_UPGRADE_RUNNING,
		TouchScreen.ACTION_UPGRADE_UNSUPPORT,
		TouchScreen.ACTION_UPGRADE_SUCCESS,
		TouchScreen.ACTION_UPGRADE_FAILED
	};

	private PreferenceScreen mPreferenceRoot;
	private PreferenceScreen mPreferenceCalibration;
	private PreferenceScreen mPreferenceTouchScreenName;
	private FirmwarePathPreference mPreferenceUpgradeFirmware;;

	private TouchScreen mTouchScreen;

	private OnPreferenceChangeListener mPreferenceChangeListener = new OnPreferenceChangeListener()
	{
		@Override
		public boolean onPreferenceChange(Preference preference, Object newValue)
		{
			Log.i(TAG, "onPreferenceChange: Preference = " + preference.getKey());
			Log.i(TAG, "newValue = " + newValue);

			if (preference == mPreferenceUpgradeFirmware)
			{
				mPreferenceUpgradeFirmware.setSelectable(false);
				new FirmwareUpgradeThread(mTouchScreen, (String) newValue).start();

				return true;
			}

			return false;
		}
	};

	private OnPreferenceClickListener mPreferenceClickListener = new OnPreferenceClickListener()
	{
		@Override
		public boolean onPreferenceClick(Preference preference)
		{
			if (preference == mPreferenceCalibration)
			{
				mPreferenceCalibration.setSelectable(false);
				new TouchScreenCalibrationThread(mTouchScreen).start();

				return true;
			}

			return false;
		}
	};

	private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver()
	{
		@Override
		public void onReceive(Context context, Intent intent)
		{
			String action = intent.getAction();
			Log.i(TAG, "action = " + action);

			if (action.equals(TouchScreen.ACTION_CALIBRATION_COUNT_DOWN))
			{
				int count = intent.getIntExtra("count", R.string.calibration_summary);
				String summary = getResources().getString(R.string.calibration_summary_start, count);
				mPreferenceCalibration.setSummary(summary);
			}
			else if (action.equals(TouchScreen.ACTION_CALIBRATION_RUNNING))
			{
				mPreferenceCalibration.setSummary(R.string.calibration_summary_running);
				mPreferenceCalibration.setSelectable(false);
			}
			else if (action.equals(TouchScreen.ACTION_CALIBRATION_SUCCESS))
			{
				mPreferenceCalibration.setSummary(R.string.calibration_summary_success);
				mPreferenceCalibration.setSelectable(true);
			}
			else if (action.equals(TouchScreen.ACTION_CALIBRATION_FAILED))
			{
				mPreferenceCalibration.setSummary(R.string.calibration_summary_failed);
				mPreferenceCalibration.setSelectable(true);
			}
			else if (action.equals(TouchScreen.ACTION_UPGRADE_RUNNING))
			{
				mPreferenceUpgradeFirmware.setSummary(R.string.upgrade_firmwire_summary_running);
				mPreferenceUpgradeFirmware.setSelectable(false);
			}
			else if (action.equalsIgnoreCase(TouchScreen.ACTION_UPGRADE_UNSUPPORT))
			{
				String summary = getResources().getString(R.string.upgrade_firmwire_summary_unsport, mTouchScreen.getDeviceName());
				mPreferenceUpgradeFirmware.setSummary(summary);
				mPreferenceUpgradeFirmware.setSelectable(true);
			}
			else if (action.equals(TouchScreen.ACTION_UPGRADE_SUCCESS))
			{
				mPreferenceUpgradeFirmware.setSummary(R.string.upgrade_firmwire_summary_success);
				mPreferenceUpgradeFirmware.setSelectable(true);
			}
			else if (action.equals(TouchScreen.ACTION_UPGRADE_FAILED))
			{
				mPreferenceUpgradeFirmware.setSummary(R.string.upgrade_firmwire_summary_failed);
				mPreferenceUpgradeFirmware.setSelectable(true);
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
		mPreferenceUpgradeFirmware = (FirmwarePathPreference) mPreferenceRoot.findPreference(KEY_UPGRADE_FIRMWIRE);

		mTouchScreen = new TouchScreen(this);

		mPreferenceCalibration.setOnPreferenceClickListener(mPreferenceClickListener);
		mPreferenceUpgradeFirmware.setOnPreferenceChangeListener(mPreferenceChangeListener);
	}

	@Override
	protected void onPause()
	{
		unregisterReceiver(mBroadcastReceiver);

		mTouchScreen.closeDevice();
		super.onPause();
	}

	@Override
	protected void onResume()
	{
		mTouchScreen.openDevice();
		mPreferenceTouchScreenName.setSummary(mTouchScreen.getDeviceName());

		IntentFilter filter = new IntentFilter();
		for (String action : mBroadcastActions)
		{
			filter.addAction(action);
		}
		registerReceiver(mBroadcastReceiver, filter);

		super.onResume();
	}
}