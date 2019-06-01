package com.cavan.accessibility;

import android.app.Notification;
import android.service.notification.StatusBarNotification;

public class CavanNotificationMM extends CavanNotification {

	public CavanNotificationMM(Notification notification) {
		super(notification);
	}

	public CavanNotificationMM(StatusBarNotification notification) {
		super(notification);
	}

	@Override
	public String getPacketName() {
		return "微信";
	}

	@Override
	public synchronized CavanAccessibilityPackage getPackage() {
		CavanAccessibilityPackage pkg = super.getPackage();
		if (pkg != null) {
			return pkg;
		}

		return CavanAccessibilityMM.instance;
	}

	@Override
	public void parse(String content) {
		int index = content.indexOf(": ");
		if (index < 0) {
			mContent = content;
		} else {
			mUserName = content.substring(0, index);
			mContent = content.substring(index + 2);
		}

		mGroupName = mTitle;
	}

	@Override
	public boolean isRedPacket() {
		if (isRedPacket("[微信红包]")) {
			return true;
		}

		if ("Windows 微信登录确认".equals(mContent)) {
			return true;
		}

		return super.isRedPacket();
	}
}
