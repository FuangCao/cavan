package com.cavan.cavanmain;

import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;

import com.cavan.cavanutils.CavanUtils;

import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;

public class MainActivity extends PreferenceActivity {

	private static final String KEY_IP_ADDRESS = "ip_address";

	private Preference mPreferenceIpAddress;

	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.cavan_service);

		mPreferenceIpAddress = findPreference(KEY_IP_ADDRESS);
		updateIpAddressStatus();
	}

	private void updateIpAddressStatus() {
		Enumeration<NetworkInterface> enNetIf;
		try {
			enNetIf = NetworkInterface.getNetworkInterfaces();
			if (enNetIf == null) {
				return;
			}
		} catch (SocketException e) {
			e.printStackTrace();
			return;
		}

		StringBuilder builder = new StringBuilder();

		while (enNetIf.hasMoreElements()) {
			Enumeration<InetAddress> enAddr = enNetIf.nextElement().getInetAddresses();
			while (enAddr.hasMoreElements()) {
				InetAddress addr = enAddr.nextElement();
				if (addr.isLoopbackAddress()) {
					continue;
				}

				if (addr.isLinkLocalAddress()) {
					continue;
				}

				if (builder.length() > 0) {
					builder.append("\n");
				}

				builder.append(addr.getHostAddress());
			}
		}

		if (builder.length() > 0) {
			mPreferenceIpAddress.setSummary(builder.toString());
		} else {
			mPreferenceIpAddress.setSummary(R.string.text_unknown);
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
