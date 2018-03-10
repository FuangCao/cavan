package com.cavan.accessibility;

import android.service.notification.StatusBarNotification;

public class CavanNotificationTimer extends CavanNotification {

	public CavanNotificationTimer(StatusBarNotification notification) {
		super(notification);

		if (mTitle != null) {
			mFinder.addLine(mTitle);
		}
	}

	@Override
	public CavanRedPacketAlipay getRedPacketAlipay(String code) {
		CavanRedPacketAlipay packet = super.getRedPacketAlipay(code);
		packet.setRepeatable();
		return packet;
	}
}
