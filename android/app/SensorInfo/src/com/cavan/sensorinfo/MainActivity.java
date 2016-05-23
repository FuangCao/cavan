package com.cavan.sensorinfo;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;

import com.cavan.cavanutils.SensorPreference;

public class MainActivity extends PreferenceActivity {

	private SensorManager mSensorManager;

	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.sensor_info);

		mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
		for (Sensor sensor : mSensorManager.getSensorList(Sensor.TYPE_ALL)) {
			SensorPreference preference = new SensorPreference(this);
			preference.init(mSensorManager, sensor);
			getPreferenceScreen().addPreference(preference);
		}
	}

	@SuppressWarnings("deprecation")
	@Override
	protected void onDestroy() {
		PreferenceScreen screen = getPreferenceScreen();
		for (int i = screen.getPreferenceCount() - 1; i >= 0; i--) {
			SensorPreference preference = (SensorPreference) screen.getPreference(i);
			preference.stop();
		}

		super.onDestroy();
	}
}
