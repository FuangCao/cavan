package com.cavan.cavanmain;
import android.app.Application;

import com.cavan.service.CavanPowerStateWatcher;

public class CavanMainApplication extends Application {

	public static CavanPowerStateWatcher gPowerStateWatcher = new CavanPowerStateWatcher();

	@Override
	public void onCreate() {
		super.onCreate();

		gPowerStateWatcher.start(this);
		FloatMessageService.startService(this);
		CavanNetworkImeConnService.startService(this);
	}
}
