package com.cavan.cavanmain;
import android.app.Application;

import com.cavan.android.CavanAndroid;


public class CavanMainApplication extends Application {

	public CavanMainApplication() {
		CavanAndroid.pLog();
	}

	@Override
	public void onCreate() {
		super.onCreate();

		FloatMessageService.startService(this);
		CavanNetworkImeConnService.startService(this);
	}
}
