package com.cavan.accessibility;

import android.app.Notification;

import com.cavan.java.CavanString;

public class CavanNotificationMM extends CavanNotification {

	public CavanNotificationMM(CavanAccessibilityPackage pkg, Notification notification) {
		super(pkg, notification);
	}

	@Override
	public void parse(String content) {
		int endLine = CavanString.findLineEnd(content);

		int index = CavanString.lastIndexOf(content, endLine, ':');
		if (index < 0) {
			mContent = content;
		} else {
			mUserName = content.substring(0, index).trim();
			mContent = content.substring(index + 1).trim();
		}

		mGroupName = mTitle;
	}

	@Override
	public boolean send() {
		if (mSendTimes > 0) {
			return false;
		}

		return super.send();
	}

	@Override
	public boolean isRedPacket() {
		return isRedPacket("[微信红包]");
	}

}
