package com.cavan.accessibility;

import java.util.List;

import android.app.Notification;
import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanString;
import com.cavan.java.RedPacketFinder;

public abstract class CavanNotification extends CavanRedPacket {

	protected Notification mNotification;
	protected String mTitle;
	protected String mContent;
	protected String mUserName;
	protected String mGroupName;

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

	public String getName() {
		if (mGroupName != null) {
			return mGroupName;
		}

		if (mUserName != null) {
			return mUserName;
		}

		return null;
	}

	protected boolean isRedPacket(String prefix) {
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
		if (mContent == null) {
			return false;
		}

		RedPacketFinder finder = new RedPacketFinder();
		finder.split(mContent);

		CavanAccessibilityAlipay alipay = CavanAccessibilityAlipay.instance;
		if (alipay != null) {
			List<String> codes = finder.getRedPacketCodes();
			if (codes != null && codes.size() > 0) {
				for (String code : codes) {
					alipay.addPacket(code);
				}

				return false;
			}
		}

		return false;
	}

	public boolean sendPendingIntent() {
		if (mNotification == null || mSendTimes > 0) {
			return false;
		}

		PendingIntent intent = mNotification.contentIntent;
		if (intent == null) {
			return false;
		}

		try {
			intent.send();
			return true;
		} catch (CanceledException e) {
			e.printStackTrace();
		}

		return false;
	}

	public abstract void parse(String content);

	@Override
	public synchronized boolean launch() {
		if (sendPendingIntent()) {
			mSendTimes++;
			return true;
		}

		return super.launch();
	}

	@Override
	public boolean needGotoIdle() {
		return true;
	}

	@Override
	public boolean equals(Object o) {
		if (super.equals(o)) {
			return true;
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
