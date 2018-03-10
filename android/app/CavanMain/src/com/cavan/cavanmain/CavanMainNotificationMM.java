package com.cavan.cavanmain;

import java.util.HashSet;

import android.content.Context;
import android.service.notification.StatusBarNotification;

import com.cavan.accessibility.CavanAccessibilityMM;
import com.cavan.accessibility.CavanNotificationMM;

public class CavanMainNotificationMM extends CavanNotificationMM {

	private static final HashSet<CharSequence> sExcludeUsers = new HashSet<CharSequence>();

	static {
		sExcludeUsers.add("微信游戏");
	}

	public CavanMainNotificationMM(StatusBarNotification notification) {
		super(notification);
	}

	@Override
	public boolean isExcluded(Context context, String user) {
		CavanAccessibilityMM mm = CavanAccessibilityMM.instance;
		if (mm != null && mm.isWebViewUi()) {
			return true;
		}

		return CavanMessageActivity.isMmFilterEnabled(context) && sExcludeUsers.contains(user);
	}

}
