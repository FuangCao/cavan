package com.cavan.resource;

import com.cavan.android.CavanAndroid;

import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.Gravity;
import android.widget.TextView;

public class CavanBleActivity extends Activity {

	protected void onCreateBle(Bundle savedInstanceState) {
		CavanAndroid.eLog("onCreateBle");
	}

	protected void onCreateNotSupport(Bundle savedInstanceState) {
		CavanAndroid.eLog("onCreateNotSupport");

		TextView view = new TextView(this);
		view.setText(R.string.ble_not_support_propmpt);
		view.setGravity(Gravity.CENTER);
		setContentView(view);
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		if (getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
			onCreateBle(savedInstanceState);
		} else {
			onCreateNotSupport(savedInstanceState);
		}
	}
}
