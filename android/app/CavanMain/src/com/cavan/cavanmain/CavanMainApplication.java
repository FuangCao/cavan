package com.cavan.cavanmain;
import android.app.Application;
import android.content.Context;

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

	public static boolean test(Context context) {
		if (!RedPacketListenerService.checkAndOpenSettingsActivity(context)) {
			FloatMessageService.showToast("请打开通知读取权限");
		} else if (!CavanMainAccessibilityService.checkAndOpenSettingsActivity(context)) {
			FloatMessageService.showToast("请打开辅助功能");
		} else {
			return true;
		}

		return false;
	}

	@Override
	public void onCreate() {
		super.onCreate();

		gPowerStateWatcher.start(this);
		FloatMessageService.startService(this);
		CavanNetworkImeConnService.startService(this);
	}
}
