package com.cavan.weixinredpacket;

import com.cavan.accessibility.CavanAccessibilityMM;
import com.cavan.accessibility.CavanAccessibilityService;

public class AccessibilityPackageMM extends CavanAccessibilityMM {

	public AccessibilityPackageMM(CavanAccessibilityService service) {
		super(service);
	}

	@Override
	public long getUnpackDelay() {
		return MainActivity.getAutoUnpackMM(mService) * 1000;
	}

}
