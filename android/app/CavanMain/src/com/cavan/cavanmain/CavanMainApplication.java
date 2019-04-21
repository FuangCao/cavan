package com.cavan.cavanmain;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.lang.Thread.UncaughtExceptionHandler;
import java.util.Calendar;

import android.app.Application;
import android.content.Context;
import android.os.Environment;

import com.cavan.android.CavanAndroid;
import com.cavan.service.CavanPowerStateListener;
import com.cavan.service.CavanPowerStateWatcher;

public class CavanMainApplication extends Application {

	private static CavanPowerStateWatcher gPowerStateWatcher = new CavanPowerStateWatcher();

	static {
		Thread.setDefaultUncaughtExceptionHandler(new UncaughtExceptionHandler() {

			@Override
			public void uncaughtException(Thread thread, Throwable ex) {
				CavanAndroid.dLog("uncaughtException: " + ex);

				File file = new File(Environment.getExternalStorageDirectory(), "cavanmain-panic.txt");
				FileOutputStream stream = null;

				try {
					stream = new FileOutputStream(file, true);
					PrintWriter writer = new PrintWriter(stream);

					writer.println("Date: " + Calendar.getInstance().getTime());
					ex.printStackTrace(writer);
					writer.close();
				} catch (Exception e) {
					e.printStackTrace();
				} finally {
					if (stream != null) {
						try {
							stream.close();
						} catch (IOException e) {
							e.printStackTrace();
						}
					}
				}
			}
		});
	}

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
		CavanInputProxyConnService.startService(this);
	}
}
