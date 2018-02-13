package com.cavan.redpacketassistant;

import android.view.accessibility.AccessibilityEvent;

import com.cavan.accessibility.CavanAccessibilityService;
import com.cavan.accessibility.CavanCountDownDialog;
import com.cavan.accessibility.CavanCountDownDialogBase;
import com.cavan.accessibility.CavanKeyguardActivity;
import com.cavan.android.CavanAndroid;

public class RedPacketAccessibilityService extends CavanAccessibilityService {

	public static RedPacketAccessibilityService instance;

	public RedPacketAccessibilityService() {
		addPackage(new RedPacketAccessibilityMM(this));
		addPackage(new RedPacketAccessibilityQQ(this));
		addPackage(new RedPacketAccessibilityAlipay(this));
	}

	@Override
	public int getEventTypes() {
		return AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED | AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED | AccessibilityEvent.TYPE_NOTIFICATION_STATE_CHANGED;
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

	@Override
	protected String getInputMethodName() {
		return getResources().getString(R.string.app_name);
	}
}
