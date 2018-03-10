package com.cavan.cavanmain;

import android.content.Context;

import com.cavan.accessibility.CavanAccessibilityAlipay;
import com.cavan.accessibility.CavanAccessibilityService;
import com.cavan.accessibility.CavanKeyguardActivity;
import com.cavan.android.CavanAndroid;

public class CavanMainAccessibilityService extends CavanAccessibilityService {

	public static CavanMainAccessibilityService instance;

	public static boolean checkAndOpenSettingsActivity(Context context) {
        if (instance != null) {
            return true;
        }

        PermissionSettingsActivity.startAccessibilitySettingsActivity(context);

        return false;
    }

	public CavanMainAccessibilityService() {
		addPackage(new CavanMainAccessibilityMM(this));
		addPackage(new CavanMainAccessibilityQQ(this));
		addPackage(new CavanAccessibilityAlipay(this));
	}

	@Override
	public Class<?> getBroadcastReceiverClass() {
		return CavanBroadcastReceiver.class;
	}

	@Override
	protected String getInputMethodName() {
		return getResources().getString(R.string.cavan_input_method);
	}

	@Override
	public void onCreate() {
		super.onCreate();
		instance = this;
	}

	@Override
	public void onDestroy() {
		instance = null;
		super.onDestroy();
	}

	@Override
	protected void onScreenOff() {
		if (CavanMessageActivity.isDisableKeyguardEnabled(this)) {
			CavanKeyguardActivity.show(this);
		} else {
			CavanAndroid.startLauncher(this);
		}
	}
}
