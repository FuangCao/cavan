package com.cavan.cavanmain;
import com.cavan.android.CavanAndroid;

import android.app.Application;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;


public class CavanMainApplication extends Application {

	private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			CavanAndroid.dLog("onReceive: " + intent.getAction());
		}
	};

	public CavanMainApplication() {
		CavanAndroid.pLog();
	}

	@Override
	public void onCreate() {
		super.onCreate();

		IntentFilter filter = new IntentFilter(Intent.ACTION_TIME_TICK);
		registerReceiver(mBroadcastReceiver, filter );
	}
}
