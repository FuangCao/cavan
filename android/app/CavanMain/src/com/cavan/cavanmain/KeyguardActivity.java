package com.cavan.cavanmain;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;

import com.cavan.android.CavanAndroid;
import com.cavan.cavanmain.R;

public class KeyguardActivity extends Activity {

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			finish();
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