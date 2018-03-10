package com.cavan.cavanmain;

import com.cavan.accessibility.CavanAccessibilityMM;
import com.cavan.accessibility.CavanAccessibilityService;

public class CavanMainAccessibilityMM extends CavanAccessibilityMM {

	public CavanMainAccessibilityMM(CavanAccessibilityService service) {
		super(service);
	}

	@Override
	public long getUnpackDelay() {
		return CavanMessageActivity.getAutoUnpackMM(mService) * 1000;
	}

}
