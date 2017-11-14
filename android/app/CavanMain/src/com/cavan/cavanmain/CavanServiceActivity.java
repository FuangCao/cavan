package com.cavan.cavanmain;

import java.net.InetAddress;

import android.content.Intent;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;

import com.cavan.android.CavanAndroid;
import com.cavan.cavanjni.CavanServicePreference;
import com.cavan.java.CavanJava;

public class CavanServiceActivity extends PreferenceActivity {

	public static final String KEY_IP_ADDRESS = "ip_address";
	public static final String KEY_SHARE_APP = "share_app";
	public static final String KEY_FTP = "ftp";
	public static final String KEY_HTTP = "http";
	public static final String KEY_TCP_DD = "tcp_dd";
	public static final String KEY_WEB_PROXY = "web_proxy";
	public static final String KEY_TCP_REPEATER = "tcp_repeater";
	public static final String KEY_MOCK_LOCATION = "mock_location";

	private Preference mPreferenceIpAddress;
	private Preference mPreferenceShareApp;
	private CavanServicePreference mPreferenceTcpDd;
	private CavanServicePreference mPreferenceFtp;
	private CavanServicePreference mPreferenceHttp;
	private CavanServicePreference mPreferenceWebProxy;
	private CavanServicePreference mPreferenceTcpRepeater;
	private CavanServicePreference mPreferenceMockLocation;

	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.service_manager);

		mPreferenceIpAddress = findPreference(KEY_IP_ADDRESS);
		mPreferenceShareApp = findPreference(KEY_SHARE_APP);
		mPreferenceFtp = (CavanServicePreference) findPreference(KEY_FTP);
		mPreferenceHttp = (CavanServicePreference) findPreference(KEY_HTTP);
		mPreferenceTcpDd = (CavanServicePreference) findPreference(KEY_TCP_DD);
		mPreferenceWebProxy = (CavanServicePreference) findPreference(KEY_WEB_PROXY);
		mPreferenceTcpRepeater = (CavanServicePreference) findPreference(KEY_TCP_REPEATER);
		mPreferenceMockLocation = (CavanServicePreference) findPreference(KEY_MOCK_LOCATION);

		updateIpAddressStatus();
	}

	@Override
	protected void onDestroy() {
		mPreferenceFtp.unbindService(this);
		mPreferenceHttp.unbindService(this);
		mPreferenceTcpDd.unbindService(this);
		mPreferenceWebProxy.unbindService(this);
		mPreferenceTcpRepeater.unbindService(this);
		mPreferenceMockLocation.unbindService(this);

		super.onDestroy();
	}

	private void updateIpAddressStatus() {
		InetAddress address = CavanJava.getIpAddress();
		if (address != null) {
			mPreferenceIpAddress.setSummary(address.getHostAddress());
		} else {
			mPreferenceIpAddress.setSummary(R.string.unknown);
		}
	}

	@SuppressWarnings("deprecation")
	@Override
	public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
		if (preference == mPreferenceIpAddress) {
			updateIpAddressStatus();
		} else if (preference == mPreferenceShareApp) {
			if (mPreferenceHttp.checkAndStart()) {
				InetAddress address = CavanJava.getIpAddress();
				String host;

				if (address != null) {
					host = address.getHostAddress();
				} else {
					host = "127.0.0.1";
				}

				String url = "http://" + host + ":" + mPreferenceHttp.getPort();
				Intent intent = new Intent(this, CavanShareAppActivity.class);
				intent.putExtra("url", url);
				startActivity(intent);
			} else {
				CavanAndroid.showToast(this, R.string.http_server_not_running);
			}
		}

		return super.onPreferenceTreeClick(preferenceScreen, preference);
	}
}
