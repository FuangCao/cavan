package com.cavan.cavanmain;
import android.app.Application;


public class CavanMainApplication extends Application {

	@Override
	public void onCreate() {
		super.onCreate();

		FloatMessageService.startService(this);
		CavanNetworkImeConnService.startService(this);
	}
}
