package com.cavan.weixinredpacket;

import com.cavan.accessibility.CavanAccessibilityMM;
import com.cavan.accessibility.CavanAccessibilityService;

public class WeixinAccessibilityPackage extends CavanAccessibilityMM {

	public WeixinAccessibilityPackage(CavanAccessibilityService service) {
		super(service);
	}

	@Override
	public long getUnpackDelay() {
		return MainActivity.getAutoUnpackMM(mService) * 1000;
	}

}
