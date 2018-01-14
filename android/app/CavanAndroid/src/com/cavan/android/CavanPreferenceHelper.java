package com.cavan.android;

import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;

public class CavanPreferenceHelper {

	@SuppressWarnings("deprecation")
	public static ListPreference findListPreference(PreferenceActivity activity, String key, OnPreferenceChangeListener listener) {
		ListPreference preference = (ListPreference) activity.findPreference(key);
		preference.setSummary(preference.getEntry());
		preference.setOnPreferenceChangeListener(listener);
		return preference;
	}

	public static boolean onPreferenceChange(Preference preference, Object newValue) {
		if (preference instanceof ListPreference) {
			ListPreference listPreference = (ListPreference) preference;
			int index = listPreference.findIndexOfValue((String) newValue);
			if (index >= 0) {
				listPreference.setSummary(listPreference.getEntries()[index]);
			}
		}

		return true;
	}
}
