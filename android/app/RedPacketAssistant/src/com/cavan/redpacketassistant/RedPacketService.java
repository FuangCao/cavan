package com.cavan.redpacketassistant;

import android.view.accessibility.AccessibilityEvent;

import com.cavan.accessibility.CavanAccessibilityPackage;
import com.cavan.accessibility.CavanAccessibilityService;
import com.cavan.android.CavanAndroid;

public class RedPacketService extends CavanAccessibilityService {

	public static RedPacketService instance;

	private CountDownDialog mCountDownDialog;

	public RedPacketService() {
		addPackage(new RedPacketMM(this));
		addPackage(new RedPacketQQ(this));
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

	@Override
	protected void onCountDownCompleted() {
		if (mCountDownDialog != null) {
			mCountDownDialog.dismiss();
			mCountDownDialog = null;
		}
	}

	@Override
	protected void onCountDownUpdated(CavanAccessibilityPackage pkg, long remain) {
		if (mCountDownDialog == null) {
			mCountDownDialog = new CountDownDialog(this);
		}

		mCountDownDialog.show(pkg, remain);
	}

}
