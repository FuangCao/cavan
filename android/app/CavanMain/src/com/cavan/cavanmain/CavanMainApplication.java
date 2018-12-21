package com.cavan.cavanmain;
import android.app.Application;

import com.cavan.service.CavanPowerStateListener;
import com.cavan.service.CavanPowerStateWatcher;

public class CavanMainApplication extends Application {

	private static CavanPowerStateWatcher gPowerStateWatcher = new CavanPowerStateWatcher();

	public static boolean isScreenOn() {
		return gPowerStateWatcher.isScreenOn();
	}

	public static boolean isUserPresent() {
		return gPowerStateWatcher.isUserPresent();
	}

	public static boolean addPowerStateListener(CavanPowerStateListener listener) {
		return gPowerStateWatcher.register(listener);
	}

	@Override
	public void onCreate() {
		super.onCreate();

		gPowerStateWatcher.start(this);
		FloatMessageService.startService(this);
		CavanNetworkImeConnService.startService(this);
	}
}
