package com.cavan.weixinredpacket;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPreferenceHelper;

public class MainActivity extends PreferenceActivity implements OnPreferenceChangeListener {

	public static final String KEY_ACCESSIBILITY = "accessibility";
	public static final String KEY_MM_UNPACK = "mm_unpack";

	public static int getAutoUnpackMM(Context context) {
		return CavanAndroid.getPreferenceInt(context, KEY_MM_UNPACK, 0);
	}

	public static Intent getAccessibilitySettingsIntent() {
		return new Intent("android.settings.ACCESSIBILITY_SETTINGS");
	}

	private Preference mPreferenceAccessibility;

	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.settings);

		mPreferenceAccessibility = findPreference(KEY_ACCESSIBILITY);
		mPreferenceAccessibility.setIntent(getAccessibilitySettingsIntent());

		CavanPreferenceHelper.findListPreference(this, KEY_MM_UNPACK, this);
	}

	@Override
	protected void onResume() {
		super.onResume();

		if (CavanAccessibilityService.instance != null) {
			mPreferenceAccessibility.setSummary(R.string.opened);
		} else if (CavanAndroid.isAccessibilityServiceEnabled(this, CavanAccessibilityService.class)) {
			mPreferenceAccessibility.setSummary(R.string.please_click_here_reopen);
		} else {
			mPreferenceAccessibility.setSummary(R.string.please_click_here_open);
		}
	}

	@Override
	public boolean onPreferenceChange(Preference preference, Object newValue) {
		return CavanPreferenceHelper.onPreferenceChange(preference, newValue);
	}
}
