package com.cavan.touchscreen;

import java.io.File;
import java.io.IOException;

import android.os.Bundle;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.test.PerformanceTestCase;
import android.text.method.Touch;
import android.util.Log;
import android.view.Menu;
import android.view.TouchDelegate;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

public class MainActivity extends PreferenceActivity implements OnPreferenceChangeListener, OnPreferenceClickListener {
	private static final String TAG = "Cavan";
	private static final String KEY_TP_DEV_NAME = "dev_name";
	private static final String KEY_TP_DEV_PATH = "dev_path";
	private static final String KEY_TP_FW_ID = "fw_id";
	private static final String KEY_TP_FW_UPGRADE = "fw_upgrade";

	private PreferenceScreen mPreferenceScreenDevName;
	private PreferenceScreen mPreferenceScreenDevPath;
	private PreferenceScreen mPreferenceScreenFwID;
	private FirmwarePathPreference mPreferenceFwUpgrade;
	private TouchscreenDevice mTouchscreenDevice;

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			Log.d(TAG, "action = " + action);

			if (action.equals(TouchscreenDevice.FW_STATE_CHANGED_ACTION)) {
				int state = intent.getIntExtra("state", -1);
				switch (state) {
				case TouchscreenDevice.FW_STATE_START:
					mPreferenceFwUpgrade.setSummary(R.string.fw_upgrade_start);
					mPreferenceScreenFwID.setEnabled(false);
					break;

				case TouchscreenDevice.FW_STATE_FAILED:
					mPreferenceFwUpgrade.setEnabled(true);
					mPreferenceScreenFwID.setEnabled(true);
					mPreferenceFwUpgrade.setSummary(R.string.fw_upgrade_failed);
					break;

				case TouchscreenDevice.FW_STATE_SUCCESS:
					mPreferenceFwUpgrade.setEnabled(true);
					mPreferenceScreenFwID.setEnabled(true);
					updateFirmwareID();
					mPreferenceFwUpgrade.setSummary(R.string.fw_upgrade_success);
					break;
				}
			}
		}
	};

	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		addPreferencesFromResource(R.xml.upgrade);

		mTouchscreenDevice = new TouchscreenDevice(this);

		mPreferenceScreenDevName = (PreferenceScreen) findPreference(KEY_TP_DEV_NAME);
		mPreferenceScreenDevName.setSummary(mTouchscreenDevice.getDevName());

		mPreferenceScreenDevPath = (PreferenceScreen) findPreference(KEY_TP_DEV_PATH);
		mPreferenceScreenDevPath.setSummary(mTouchscreenDevice.getDevPath());

		mPreferenceScreenFwID = (PreferenceScreen) findPreference(KEY_TP_FW_ID);
		mPreferenceScreenFwID.setOnPreferenceClickListener(this);
		updateFirmwareID();

		mPreferenceFwUpgrade = (FirmwarePathPreference) findPreference(KEY_TP_FW_UPGRADE);
		String pathname = Settings.System.getString(getContentResolver(), KEY_TP_FW_UPGRADE);
		if (pathname != null) {
			File file = new File(pathname);
			if (file.exists()) {
				mPreferenceFwUpgrade.setText(pathname);
			}
		}

		mPreferenceFwUpgrade.setOnPreferenceChangeListener(this);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}

	@Override
	protected void onPause() {
		unregisterReceiver(mReceiver);
		super.onPause();
	}

	@Override
	protected void onResume() {
		IntentFilter filter = new IntentFilter(TouchscreenDevice.FW_STATE_CHANGED_ACTION);
		registerReceiver(mReceiver, filter);
		super.onResume();
	}

	@Override
	public boolean onPreferenceChange(Preference preference, Object newValue) {
		if (preference.equals(mPreferenceFwUpgrade)) {
			mPreferenceFwUpgrade.setEnabled(false);
			mTouchscreenDevice.upgradeFirmware(newValue.toString());
			Settings.System.putString(getContentResolver(), KEY_TP_FW_UPGRADE, newValue.toString());
			return true;
		}

		return false;
	}

	@Override
	public boolean onPreferenceClick(Preference preference) {
		if (preference.equals(mPreferenceScreenFwID)) {
			updateFirmwareID();
		}

		return false;
	}

	private void updateFirmwareID() {
		mPreferenceScreenFwID.setSummary(String.format("0x%04x", mTouchscreenDevice.readFwID()));
	}
}