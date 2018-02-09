package com.cavan.redpacketassistant;

import android.view.accessibility.AccessibilityEvent;

import com.cavan.accessibility.CavanAccessibilityService;
import com.cavan.accessibility.CavanRedPacket;
import com.cavan.android.CavanAndroid;

public class RedPacketAccessibilityService extends CavanAccessibilityService {

	public static RedPacketAccessibilityService instance;

	private CountDownDialog mCountDownDialog;

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
	protected void onUserOffline() {
		CavanAndroid.startLauncher(this);
	}

	@Override
	protected void onCountDownCompleted() {
		if (mCountDownDialog != null) {
			mCountDownDialog.dismiss();
			mCountDownDialog = null;
		}
	}

	@Override
	protected void onCountDownUpdated(CavanRedPacket packet, long remain) {
		if (mCountDownDialog == null) {
			mCountDownDialog = new CountDownDialog(this);
		}

		mCountDownDialog.show(packet, remain);
	}

}
