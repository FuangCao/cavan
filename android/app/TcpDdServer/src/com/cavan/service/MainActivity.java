package com.cavan.service;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.util.Log;
import android.view.Menu;

import com.cavan.service.TcpDdService.ServiceBinder;

public class MainActivity extends PreferenceActivity implements OnPreferenceChangeListener {
	private static final String TAG = "Cavan";
	private static final String KEY_ENABLE_SERVICE = "enable_service";
	private static final String KEY_SERVICE_PORT = "service_port";

	private CheckBoxPreference mPreferenceEnableService;
	private EditTextPreference mPreferenceServicePort;

	private TcpDdService mService;
	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceDisconnected(ComponentName arg0) {
			Log.d(TAG, "onServiceDisconnected");
			mService = null;
		}

		@Override
		public void onServiceConnected(ComponentName arg0, IBinder arg1) {
			Log.d(TAG, "onServiceConnected");
			TcpDdService.ServiceBinder binder = (ServiceBinder) arg1;
			mService = binder.getService();
			updateServiceState(mService.getState());

			String port = Integer.toString(mService.getPort());
			mPreferenceServicePort.setText(port);
			mPreferenceServicePort.setSummary(port);
		}
	};

	private IntentFilter mFilter;

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {
		@Override
			public void onReceive(Context arg0, Intent arg1) {
				String action = arg1.getAction();
				Log.d(TAG, "action = " + action);

				if (action.equals(TcpDdService.ACTION_SERVICE_STATE_CHANGE)) {
					int state = arg1.getIntExtra("state", -1);
					updateServiceState(state);
			}
		}
	};

	private void updateServiceState(int state) {
		switch (state) {
		case TcpDdService.SERVICE_STATE_RUNNING:
			mPreferenceEnableService.setChecked(true);
			mPreferenceEnableService.setEnabled(true);
			mPreferenceEnableService.setSummary(R.string.service_running);
			mPreferenceServicePort.setEnabled(false);
			break;
		case TcpDdService.SERVICE_STATE_STOPPED:
			mPreferenceEnableService.setChecked(false);
			mPreferenceEnableService.setEnabled(true);
			mPreferenceEnableService.setSummary(R.string.service_stopped);
			mPreferenceServicePort.setEnabled(true);
			break;
		}
	}

	public MainActivity() {
		super();

		mFilter = new IntentFilter();
		mFilter.addAction(TcpDdService.ACTION_SERVICE_STATE_CHANGE);
	}

	@Override
	protected void onPause() {
		unregisterReceiver(mReceiver);
		super.onPause();
	}

	@Override
	protected void onResume() {
		registerReceiver(mReceiver, mFilter);
		super.onResume();
	}

	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.main);
		mPreferenceEnableService = (CheckBoxPreference) findPreference(KEY_ENABLE_SERVICE);
		mPreferenceEnableService.setOnPreferenceChangeListener(this);

		mPreferenceServicePort = (EditTextPreference) findPreference(KEY_SERVICE_PORT);
		mPreferenceServicePort.setOnPreferenceChangeListener(this);

		Intent service = new Intent(this, TcpDdService.class);
		bindService(service, mConnection, BIND_AUTO_CREATE);
	}

	@Override
	protected void onDestroy() {
		Log.d(TAG, "onDestroy()");
		super.onDestroy();
		unbindService(mConnection);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}

	public boolean onPreferenceChange(Preference arg0, Object arg1) {
		if (arg0.equals(mPreferenceEnableService)) {
			mPreferenceEnableService.setEnabled(false);
			if (arg1.equals(true)) {
				mPreferenceEnableService.setSummary(R.string.service_opening);
				int port = Integer.valueOf(mPreferenceServicePort.getText());
				mService.start(port);
			} else {
				mPreferenceEnableService.setSummary(R.string.service_stopping);
				mService.stop();
			}
		} else if (arg0.equals(mPreferenceServicePort)) {
			mPreferenceServicePort.setSummary((CharSequence) arg1);
			return true;
		}

		return false;
	}
}
