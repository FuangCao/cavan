package com.cavan.accessibility;

import android.app.Notification;
import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanString;

public abstract class CavanNotification extends CavanRedPacket {

	protected Notification mNotification;
	protected String mTitle;
	protected String mContent;
	protected String mUserName;
	protected String mGroupName;
	protected int mSendTimes;

	public CavanNotification(CavanAccessibilityPackage pkg, Notification notification) {
		super(pkg);

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
			CavanAndroid.dLog(content);
			parse(content);
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

	public boolean isRedPacket(String prefix) {
		if (mContent == null) {
			return false;
		}

		if (!mContent.startsWith(prefix)) {
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

	public boolean isRedPacket() {
		return false;
	}

	public abstract void parse(String content);

	public boolean send() {
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
