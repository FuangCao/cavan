package com.cavan.cavanmain;

import java.net.InetAddress;

import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;

import com.cavan.cavanjni.CavanServicePreference;
import com.cavan.cavanmain.R;
import com.cavan.java.CavanJava;

public class CavanServiceActivity extends PreferenceActivity {

	public static final String KEY_IP_ADDRESS = "ip_address";
	public static final String KEY_FTP = "ftp";
	public static final String KEY_HTTP = "http";
	public static final String KEY_TCP_DD = "tcp_dd";
	public static final String KEY_WEB_PROXY = "web_proxy";
	public static final String KEY_TCP_REPEATER = "tcp_repeater";

	private Preference mPreferenceIpAddress;
	private CavanServicePreference mPreferenceTcpDd;
	private CavanServicePreference mPreferenceFtp;
	private CavanServicePreference mPreferenceHttp;
	private CavanServicePreference mPreferenceWebProxy;
	private CavanServicePreference mPreferenceTcpRepeater;

	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.service_manager);

		mPreferenceIpAddress = findPreference(KEY_IP_ADDRESS);
		mPreferenceFtp = (CavanServicePreference) findPreference(KEY_FTP);
		mPreferenceHttp = (CavanServicePreference) findPreference(KEY_HTTP);
		mPreferenceTcpDd = (CavanServicePreference) findPreference(KEY_TCP_DD);
		mPreferenceWebProxy = (CavanServicePreference) findPreference(KEY_WEB_PROXY);
		mPreferenceTcpRepeater = (CavanServicePreference) findPreference(KEY_TCP_REPEATER);

		updateIpAddressStatus();
	}

	@Override
	protected void onDestroy() {
		mPreferenceFtp.unbindService(this);
		mPreferenceHttp.unbindService(this);
		mPreferenceTcpDd.unbindService(this);
		mPreferenceWebProxy.unbindService(this);
		mPreferenceTcpRepeater.unbindService(this);

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
		}

		return super.onPreferenceTreeClick(preferenceScreen, preference);
	}
}
