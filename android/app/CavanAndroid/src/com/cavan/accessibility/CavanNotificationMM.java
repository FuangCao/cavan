package com.cavan.accessibility;

import android.app.Notification;
import android.service.notification.StatusBarNotification;

public class CavanNotificationMM extends CavanNotification {

	private String mPacketName = "微信";

	public CavanNotificationMM(Notification notification) {
		super(notification);
	}

	public CavanNotificationMM(StatusBarNotification notification) {
		super(notification);
	}

	@Override
	public String getPacketName() {
		return mPacketName;
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
			mPacketName = "登录确认";
			return true;
		}

		return super.isRedPacket();
	}
}
