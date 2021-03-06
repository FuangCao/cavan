package com.cavan.touchscreen;

import java.io.File;
import java.util.List;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.util.Log;
import android.view.Menu;
import android.widget.Toast;

public class MainActivity extends PreferenceActivity implements OnPreferenceChangeListener, OnPreferenceClickListener {
	private static final String TAG = "Cavan";
	private static final String KEY_TP_DEV_NAME = "dev_name";
	private static final String KEY_TP_DEV_PATH = "dev_path";
	private static final String KEY_TP_FW_ID = "fw_id";
	private static final String KEY_TP_FW_NAME = "fw_name";
	private static final String KEY_TP_VENDOR_NAME = "vendor_name";
	private static final String KEY_TP_FW_UPGRADE = "fw_upgrade";
	private static final String KEY_TP_FW_LIST = "fw_list";

	private PreferenceScreen mPreferenceScreenDevName;
	private PreferenceScreen mPreferenceScreenDevPath;
	private PreferenceScreen mPreferenceScreenFwID;
	private PreferenceScreen mPreferenceScreenFwName;
	private PreferenceScreen mPreferenceScreenVendorName;
	private FirmwarePathPreference mPreferenceFwUpgrade;
	private PreferenceCategory mPreferenceCategoryFwList;

	private ITouchscreenService mService;
	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceDisconnected(ComponentName name) {
			mService = null;
		}

		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			mService = ITouchscreenService.Stub.asInterface(service);

			try {
				mPreferenceScreenDevName.setSummary(mService.getDevName());
				mPreferenceScreenDevPath.setSummary(mService.getDevPath());
				updateFirmwareID();
			} catch (RemoteException e) {
				e.printStackTrace();
			}

			scanFirmware();
		}
	};

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			Log.d(TAG, "action = " + action);

			if (action.equals(TouchscreenService.FW_STATE_CHANGED_ACTION)) {
				int state = intent.getIntExtra("state", -1);
				switch (state) {
				case TouchscreenService.FW_STATE_START:
					mPreferenceFwUpgrade.setSummary(R.string.fw_upgrade_start);
					mPreferenceScreenFwID.setEnabled(false);
					mPreferenceScreenFwName.setEnabled(false);
					mPreferenceScreenVendorName.setEnabled(false);
					break;

				case TouchscreenService.FW_STATE_RETRY:
					mPreferenceFwUpgrade.setSummary(R.string.fw_upgrade_retry);
					break;

				case TouchscreenService.FW_STATE_FAILED:
					mPreferenceFwUpgrade.setEnabled(true);
					mPreferenceScreenFwID.setEnabled(true);
					mPreferenceScreenFwName.setEnabled(true);
					mPreferenceScreenVendorName.setEnabled(true);
					mPreferenceFwUpgrade.setSummary(R.string.fw_upgrade_failed);
					break;

				case TouchscreenService.FW_STATE_SUCCESS:
					mPreferenceFwUpgrade.setEnabled(true);
					mPreferenceScreenFwID.setEnabled(true);
					mPreferenceScreenFwName.setEnabled(true);
					mPreferenceScreenVendorName.setEnabled(true);
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
		// setContentView(R.layout.activity_main);
		addPreferencesFromResource(R.xml.upgrade);

		mPreferenceScreenDevName = (PreferenceScreen) findPreference(KEY_TP_DEV_NAME);
		mPreferenceScreenDevPath = (PreferenceScreen) findPreference(KEY_TP_DEV_PATH);
		mPreferenceScreenFwID = (PreferenceScreen) findPreference(KEY_TP_FW_ID);
		mPreferenceScreenFwID.setOnPreferenceClickListener(this);
		mPreferenceScreenFwName = (PreferenceScreen) findPreference(KEY_TP_FW_NAME);
		mPreferenceScreenFwName.setOnPreferenceClickListener(this);
		mPreferenceScreenVendorName = (PreferenceScreen) findPreference(KEY_TP_VENDOR_NAME);
		mPreferenceScreenVendorName.setOnPreferenceClickListener(this);

		mPreferenceFwUpgrade = (FirmwarePathPreference) findPreference(KEY_TP_FW_UPGRADE);
		String pathname = Settings.System.getString(getContentResolver(), KEY_TP_FW_UPGRADE);
		if (pathname != null) {
			File file = new File(pathname);
			if (file.exists()) {
				mPreferenceFwUpgrade.setText(pathname);
			}
		}

		mPreferenceFwUpgrade.setOnPreferenceChangeListener(this);

		mPreferenceCategoryFwList = (PreferenceCategory) findPreference(KEY_TP_FW_LIST);

		Intent service = new Intent(this, TouchscreenService.class);
		bindService(service, mConnection, BIND_AUTO_CREATE);
	}

	private void scanFirmware() {
		Thread thread = new Thread() {
			public void run() {
				mPreferenceCategoryFwList.removeAll();

				List<String> listFw;
				String fwName;

				try {
					fwName = mService.getFwName();
					if (fwName == null) {
						return;
					}

					listFw = mService.findFirmware();
					if (listFw == null) {
						return;
					}
				} catch (RemoteException e) {
					e.printStackTrace();
					return;
				}

				for (String fwPath : listFw) {
					Log.d(TAG, "pathname = " + fwPath);
					Preference preference = new Preference(getApplicationContext());
					preference.setTitle(fwName);
					preference.setSummary(fwPath);
					preference.setEnabled(true);
					mPreferenceCategoryFwList.addPreference(preference);
				}
			}
		};

		thread.start();
	}

	@Override
	protected void onDestroy() {
		unbindService(mConnection);
		super.onDestroy();
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
		IntentFilter filter = new IntentFilter(TouchscreenService.FW_STATE_CHANGED_ACTION);
		registerReceiver(mReceiver, filter);
		super.onResume();
	}

	@Override
	public boolean onPreferenceChange(Preference preference, Object newValue) {
		if (preference.equals(mPreferenceFwUpgrade)) {
			if (mService != null) {
				mPreferenceFwUpgrade.setEnabled(false);

				try {
					mService.upgradeFirmware(newValue.toString());
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}

			Settings.System.putString(getContentResolver(), KEY_TP_FW_UPGRADE, newValue.toString());
			return true;
		}

		return false;
	}

	@Override
	public boolean onPreferenceClick(Preference preference) {
		if (preference.equals(mPreferenceScreenFwID) || preference.equals(mPreferenceScreenFwName) || preference.equals(mPreferenceScreenVendorName)) {
			updateFirmwareID();
		}

		return false;
	}

	private void updateFirmwareID() {
		if (mService == null) {
			return;
		}

		try {
			DeviceID devID = mService.readDevID();
			if (devID == null) {
				return;
			}

			mPreferenceScreenFwID.setSummary(String.format("%02x%02x", devID.getVendorID(), devID.getFwVersion()));
			mPreferenceScreenVendorName.setSummary(devID.getVendorNameResId());
			mPreferenceScreenFwName.setSummary(mService.getFwName());
		} catch (RemoteException e) {
			e.printStackTrace();
		}
	}
}