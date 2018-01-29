package com.cavan.accessibility;

import com.cavan.java.CavanString;

import android.app.Notification;

public class CavanNotificationQQ extends CavanNotification {

	public CavanNotificationQQ(CavanAccessibilityPackage pkg, Notification notification) {
		super(pkg, notification);
	}

	@Override
	public void parse(String content) {
		int endLine = CavanString.findLineEnd(content);
		int index = content.indexOf("):");

		if (index < 0 || index > endLine) {
			index = CavanString.lastIndexOf(content, endLine, ':');
			if (index < 0) {
				mContent = content;
			} else {
				mUserName = content.substring(0, index).trim();
				mContent = content.substring(index + 1);
			}
		} else {
			int group = index - 1;

			for (int count = 1; group >= 0; group--){
				char c = content.charAt(group);

				if (c == ')') {
					count++;
				} else if (c == '(') {
					if (--count <= 0) {
						break;
					}
				}
			}

			if (group < 0) {
				mUserName = content.substring(0, index).trim();
			} else {
				mUserName = content.substring(0, group).trim();
				mGroupName = content.substring(group + 1, index).trim();
			}

			mContent = content.substring(index + 2);
		}
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
		return isRedPacket("[QQ红包]");
	}

}
