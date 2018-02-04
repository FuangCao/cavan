package com.cavan.accessibility;

import android.app.Notification;

import com.cavan.java.CavanString;

public class CavanNotificationMM extends CavanNotification {

	public CavanNotificationMM(Notification notification) {
		super(notification);
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
	public boolean isRedPacket() {
		if (isRedPacket("[微信红包]")) {
			return true;
		}

		return super.isRedPacket();
	}

}
