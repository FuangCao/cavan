package com.cavan.accessibility;

import android.app.Notification;
import android.service.notification.StatusBarNotification;

import com.cavan.java.CavanString;

public class CavanNotificationQQ extends CavanNotification {

	public CavanNotificationQQ(Notification notification) {
		super(notification);
	}

	public CavanNotificationQQ(StatusBarNotification notification) {
		super(notification);
	}

	@Override
	public String getPacketName() {
		return "QQ";
	}

	@Override
	public synchronized CavanAccessibilityPackage getPackage() {
		CavanAccessibilityPackage pkg = super.getPackage();
		if (pkg != null) {
			return pkg;
		}

		return CavanAccessibilityQQ.instance;
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
				mContent = content.substring(index + 2);
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
	public boolean isRedPacket() {
		if (isRedPacket("[QQ红包]")) {
			return true;
		}

		return super.isRedPacket();
	}

	@Override
	public long getCodeDelay() {
		if (mGroupName != null) {
			if (mGroupName.equals("【VIP】内部福利6群")) {
				return 8000;
			}

			if (mGroupName.equals("【小六04】内部VIP群")) {
				return 10000;
			}

			if (mGroupName.equals("11-VIP客户内部福利群")) {
				return 5000;
			}
		}

		return super.getCodeDelay();
	}
}
