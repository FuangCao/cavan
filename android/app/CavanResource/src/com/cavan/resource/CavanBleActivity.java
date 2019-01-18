package com.cavan.resource;

import android.app.Activity;
import android.os.Bundle;
import android.view.Gravity;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanBluetoothAdapter;

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

		if (CavanBluetoothAdapter.hasBleHardware(this)) {
			onCreateBle(savedInstanceState);
		} else {
			onCreateNotSupport(savedInstanceState);
		}
	}
}
