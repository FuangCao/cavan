package com.cavan.weixinredpacket;

import com.cavan.accessibility.CavanAccessibilityService;

public class WeixinRedPacketService extends CavanAccessibilityService {

	public static WeixinRedPacketService instance;

	public WeixinRedPacketService() {
		addPackage(new CavanAccessibilityMM(this));
	}

	@Override
	public void onCreate() {
		super.onCreate();
		instance = this;
	}

	@Override
	public void onDestroy() {
		instance = null;
		super.onDestroy();
	}

}
