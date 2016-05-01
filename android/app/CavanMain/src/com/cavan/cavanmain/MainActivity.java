package com.cavan.cavanmain;

import java.util.HashMap;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;

import com.cavan.cavanutils.CavanUtils;

public class MainActivity extends PreferenceActivity {

	public static final String TAG = "Cavan";

	private static final String KEY_START_STOP = "start_stop_service";
	private static final String[] KEY_SERVICE_LIST = {
		"ftp_service", "tcp_dd_service", "web_proxy_service"
	};

	private ICavanService mService;

	private Preference mPreferenceStartStop;
	private HashMap<String, CavanServicePreference> mHashMapPreference = new HashMap<String, CavanServicePreference>();

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			CavanUtils.logE("action = " + action);

			for (CavanServicePreference preference : mHashMapPreference.values()) {
				if (preference.getAction().equals(action)) {
					boolean state = intent.getBooleanExtra("state", false);
					preference.updateSummary(state);
					break;
				}
			}
		}
	};

	private ServiceConnection mConnection = new ServiceConnection() {

		@Override
		public void onServiceDisconnected(ComponentName arg0) {
			CavanUtils.logE("onServiceDisconnected");
			setService(null);
		}

		@Override
		public void onServiceConnected(ComponentName arg0, IBinder arg1) {
			CavanUtils.logE("onServiceConnected");
			ICavanService service = ICavanService.Stub.asInterface(arg1);
			setService(service);
		}
	};

	public void setService(ICavanService service) {
		mService = service;

		for (CavanServicePreference preference : mHashMapPreference.values()) {
			preference.setService(service);
		}

		if (service == null) {
			mPreferenceStartStop.setTitle(R.string.text_start_service);
			mPreferenceStartStop.setSummary(R.string.text_service_disconnected);
		} else {
			mPreferenceStartStop.setTitle(R.string.text_close_service);
			mPreferenceStartStop.setSummary(R.string.text_service_connected);
		}
	}

	private void startService(boolean enable) {
		Intent service = new Intent(this, CavanService.class);
		if (enable) {
			bindService(service, mConnection, 0); // BIND_AUTO_CREATE);
			startService(service);
		} else {
			stopService(service);
		}
	}

	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.cavan_service);

		for (String key : KEY_SERVICE_LIST) {
			CavanServicePreference preference = (CavanServicePreference) findPreference(key);
			if (preference == null) {
				continue;
			}

			CavanUtils.logE("key = " + key + ", preference = " + preference);

			mHashMapPreference.put(key, preference);
		}

		mPreferenceStartStop = findPreference(KEY_START_STOP);
		startService(true);
	}

	@Override
	protected void onDestroy() {
		unbindService(mConnection);
		super.onDestroy();
	}

	@Override
	protected void onPause() {
		unregisterReceiver(mReceiver);
		super.onPause();
	}

	@Override
	protected void onResume() {
		IntentFilter filter = new IntentFilter();

		for (CavanServicePreference preference : mHashMapPreference.values()) {
			filter.addAction(preference.getAction());
		}

		registerReceiver(mReceiver, filter);

		super.onResume();
	}

	@SuppressWarnings("deprecation")
	@Override
	public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
			Preference preference) {

		String key = preference.getKey();
		if (key.equals(KEY_START_STOP)) {
			startService(mService == null);
		}

		return super.onPreferenceTreeClick(preferenceScreen, preference);
	}
}
