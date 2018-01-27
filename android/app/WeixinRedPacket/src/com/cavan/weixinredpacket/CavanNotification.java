package com.cavan.weixinredpacket;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanString;

import android.app.Notification;
import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;

public class CavanNotification {

	private Notification mNotification;
	private String mTitle;
	private String mContent;
	private String mUserName;
	private String mGroupName;
	private int mSendTimes;

	public CavanNotification(Notification notification) {
		mNotification = notification;

		CharSequence title = notification.extras.getCharSequence(Notification.EXTRA_TITLE);
		if (title != null) {
			mTitle = title.toString();
		}

		CavanAndroid.dLog("[" + mTitle + "] ================================================================================");

		CharSequence text = notification.tickerText;
		if (text == null) {
			text = notification.extras.getCharSequence(Notification.EXTRA_TEXT);
		}

		if (text != null) {
			String content = text.toString();
			int endLine = CavanString.findLineEnd(content);

			CavanAndroid.dLog(content);

			int index = CavanString.lastIndexOf(content, endLine, ':');
			if (index < 0) {
				mContent = content;
			} else {
				mUserName = content.substring(0, index).trim();
				mContent = content.substring(index + 1).trim();
			}

			mGroupName = mTitle;
		}
	}

	public Notification getNotification() {
		return mNotification;
	}

	public String getTitle() {
		return mTitle;
	}

	public String getContent() {
		return mContent;
	}

	public String getUserName() {
		if (mUserName == null) {
			return "unknown";
		}

		return mUserName;
	}

	public String getGroupName() {
		return mGroupName;
	}

	public boolean isRedPacket() {
		if (mContent == null) {
			return false;
		}

		if (!mContent.startsWith("[微信红包]")) {
			return false;
		}

		if (mContent.contains("测") || mContent.contains("挂")) {
			return false;
		}

		if (CavanString.getLineCount(mContent) > 1) {
			return false;
		}

		return true;
	}

	public boolean send() {
		if (mSendTimes > 0) {
			return false;
		}

		PendingIntent intent = mNotification.contentIntent;
		if (intent == null) {
			return false;
		}

		try {
			intent.send();
			mSendTimes++;
		} catch (CanceledException e) {
			e.printStackTrace();
			return false;
		}

		return true;
	}

	@Override
	public boolean equals(Object o) {
		if (o == null) {
			return false;
		}

		if (o instanceof CavanNotification) {
			CavanNotification notification = (CavanNotification) o;
			return getUserName().equals(notification.getUserName());
		}

		if (o instanceof String) {
			return getUserName().equals(o);
		}

		return false;
	}
}
