package com.cavan.cavanmain;

import com.cavan.accessibility.CavanAccessibilityQQ;
import com.cavan.accessibility.CavanAccessibilityService;

public class CavanMainAccessibilityQQ extends CavanAccessibilityQQ {

	public CavanMainAccessibilityQQ(CavanAccessibilityService service) {
		super(service);
	}

	@Override
	public long getUnpackDelay() {
		return CavanMessageActivity.getAutoUnpackQQ(mService) * 1000;
	}
}
