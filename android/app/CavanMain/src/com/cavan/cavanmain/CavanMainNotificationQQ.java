package com.cavan.cavanmain;

import java.util.HashSet;

import android.content.Context;
import android.service.notification.StatusBarNotification;

import com.cavan.accessibility.CavanNotificationQQ;

public class CavanMainNotificationQQ extends CavanNotificationQQ {

	private static final HashSet<CharSequence> sExcludeUsers = new HashSet<CharSequence>();

	static {
		sExcludeUsers.add("QQ钱包");
	}

	public CavanMainNotificationQQ(StatusBarNotification notification) {
		super(notification);
	}

	@Override
	public boolean isExcluded(Context context, String user) {
		return CavanMessageActivity.isQqFilterEnabled(context) && sExcludeUsers.contains(user);
	}
}
