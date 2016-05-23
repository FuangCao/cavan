package com.cavan.batteryinfo;

import java.io.File;

import android.os.Bundle;
import android.preference.PreferenceActivity;

import com.cavan.cavanutils.PowerSupplyPreferenceCategory;

public class MainActivity extends PreferenceActivity {

	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.battery_info);

		File dirPowerSupply = new File("/sys/class/power_supply");
		for (File dir : dirPowerSupply.listFiles()) {
			PowerSupplyPreferenceCategory category = new PowerSupplyPreferenceCategory(this);
			getPreferenceScreen().addPreference(category);
			category.init(dir);
			category.setAutoUpdate(1000);
		}
	}
}
