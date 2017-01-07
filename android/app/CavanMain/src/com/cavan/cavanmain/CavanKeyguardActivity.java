package com.cavan.cavanmain;

import com.cavan.android.CavanAndroid;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;

public class CavanKeyguardActivity extends Activity {

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			if (MainActivity.isAutoUnlockEnabled(getApplicationContext())) {
				finish();
			}
		}
	};

	@Override
	protected void onCreate(Bundle bundle) {
		super.onCreate(bundle);
		setContentView(R.layout.keyguard);

		CavanAndroid.setActivityKeyguardEnable(this, false);
		registerReceiver(mReceiver, new IntentFilter(Intent.ACTION_SCREEN_ON));
	}

	@Override
	protected void onDestroy() {
		unregisterReceiver(mReceiver);
		super.onDestroy();
	}
}