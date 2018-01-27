package com.cavan.weixinredpacket;

import android.view.accessibility.AccessibilityEvent;

import com.cavan.accessibility.CavanAccessibilityService;
import com.cavan.android.CavanAndroid;

public class WeixinRedPacketService extends CavanAccessibilityService {

	public static WeixinRedPacketService instance;

	public WeixinRedPacketService() {
		addPackage(new CavanAccessibilityMM(this));
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
	protected void onUserOffline() {
		CavanAndroid.startLauncher(this);
	}

}
