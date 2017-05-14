package com.cavan.cavanmain;

import java.util.HashMap;

import android.content.Intent;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;

import com.cavan.android.CavanAndroid;

@SuppressWarnings("deprecation")
public class CavanApplicationActivity extends PreferenceActivity {

	private static final HashMap<String, Class<?>> sHashMapKeyActivity = new HashMap<String, Class<?>>();

	static {
		sHashMapKeyActivity.put("text_search", com.cavan.textsearch.MainActivity.class);
		sHashMapKeyActivity.put("apk_backup", com.cavan.apkbackup.MainActivity.class);
		sHashMapKeyActivity.put("qr_code_encode", com.cavan.qrcode.QrCodeEncodeActivity.class);
		sHashMapKeyActivity.put("qrcode_decode", com.cavan.qrcode.QrCodeDecodeActivity.class);
		sHashMapKeyActivity.put("radix_converter", com.cavan.radixconverter.MainActivity.class);
		sHashMapKeyActivity.put("resistor_decoder", com.cavan.resistordecoder.MainActivity.class);
		sHashMapKeyActivity.put("wifi_detector", com.cavan.wifi.WifiDetectorActivity.class);
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.application_manager);

		PreferenceScreen screen = getPreferenceScreen();
		if (screen != null) {
			for (int i = screen.getPreferenceCount() - 1; i >= 0; i--) {
				Preference preference = screen.getPreference(i);

				Class<?> cls = sHashMapKeyActivity.get(preference.getKey());
				if (cls != null) {
					Intent intent = new Intent(this, cls);
					intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
					preference.setIntent(intent);
				} else {
					CavanAndroid.eLog("Invalid preference: " + preference);
				}
			}
		}
	}
}
