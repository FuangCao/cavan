package com.cavan.accessibility;

import android.app.Activity;
import android.app.KeyguardManager;
import android.content.Context;
import android.os.Bundle;

import com.cavan.android.CavanAndroid;

public class CavanUnlockActivity extends Activity implements Runnable {

	public static CavanUnlockActivity instance;
	public static long lasttime;

	public static boolean setLockScreenEnable(Context context, boolean enable) {
		KeyguardManager manager = (KeyguardManager) CavanAndroid.getSystemServiceCached(context, KEYGUARD_SERVICE);
		if (manager == null) {
			return false;
		}

		if (enable || CavanAndroid.SDK_VERSION < CavanAndroid.SDK_VERSION_80) {
			return CavanAndroid.setLockScreenEnable(manager, enable);
		}

		if (instance != null) {
			return true;
		}

		enable = CavanAndroid.isLockScreenEnabled(manager);
		CavanAndroid.dLog("isLockScreenEnabled: " + enable);

		if (enable) {
			long time = System.currentTimeMillis();
			if (time - lasttime < 2000) {
				return true;
			}

			lasttime = time;

			return CavanAndroid.startActivity(context, CavanUnlockActivity.class);
		}

		return true;
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		instance = this;

		CavanAndroid.setActivityKeyguardEnable(this, false, true);
		CavanAndroid.requestDismissKeyguard(this);
		CavanAndroid.postRunnable(this);
	}

	@Override
	protected void onDestroy() {
		instance = null;
		super.onDestroy();
	}

	@Override
	public void run() {
		finish();
	}
}
