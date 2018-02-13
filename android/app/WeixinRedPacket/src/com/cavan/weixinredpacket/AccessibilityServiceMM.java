package com.cavan.weixinredpacket;

import android.view.accessibility.AccessibilityEvent;

import com.cavan.accessibility.CavanAccessibilityService;
import com.cavan.accessibility.CavanCountDownDialog;
import com.cavan.accessibility.CavanCountDownDialogBase;
import com.cavan.accessibility.CavanKeyguardActivity;
import com.cavan.android.CavanAndroid;

public class AccessibilityServiceMM extends CavanAccessibilityService {

	public static AccessibilityServiceMM instance;

	public AccessibilityServiceMM() {
		addPackage(new AccessibilityPackageMM(this));
	}

	@Override
	public int getEventTypes() {
		return AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED | AccessibilityEvent.TYPE_NOTIFICATION_STATE_CHANGED;
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
		if (MainActivity.isDisableKeyguardEnabled(this)) {
			CavanKeyguardActivity.show(this);
		} else {
			CavanAndroid.startLauncher(this);
		}
	}

	@Override
	protected CavanCountDownDialogBase createCountDownDialog() {
		return new CavanCountDownDialog(this);
	}
}
