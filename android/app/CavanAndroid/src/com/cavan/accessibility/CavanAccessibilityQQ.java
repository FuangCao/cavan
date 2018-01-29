package com.cavan.accessibility;

import android.app.Notification;

import com.cavan.android.CavanPackageName;

public class CavanAccessibilityQQ extends CavanAccessibilityPackage {

	public CavanAccessibilityQQ(CavanAccessibilityService service) {
		super(service);
	}

	@Override
	public String getPackageName() {
		return CavanPackageName.QQ;
	}

	@Override
	public void initWindows() {
	}

	@Override
	public synchronized void onNotificationStateChanged(Notification notification) {
		CavanNotificationQQ packet = new CavanNotificationQQ(this, notification);
		if (packet.isRedPacket()) {
			addPacket(packet);
		}
	}

}
