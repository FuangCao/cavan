package com.cavan.redpacketassistant;

import com.cavan.accessibility.CavanAccessibilityMM;
import com.cavan.accessibility.CavanAccessibilityService;

public class RedPacketAccessibilityMM extends CavanAccessibilityMM {

	public RedPacketAccessibilityMM(CavanAccessibilityService service) {
		super(service);
	}

	@Override
	public long getUnpackDelay() {
		return MainActivity.getAutoUnpackMM(mService) * 1000;
	}

}
