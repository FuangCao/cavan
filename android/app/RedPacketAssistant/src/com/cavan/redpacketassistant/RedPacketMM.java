package com.cavan.redpacketassistant;

import com.cavan.accessibility.CavanAccessibilityMM;
import com.cavan.accessibility.CavanAccessibilityService;

public class RedPacketMM extends CavanAccessibilityMM {

	public RedPacketMM(CavanAccessibilityService service) {
		super(service);
	}

	@Override
	public long getUnpackDelay() {
		return MainActivity.getAutoUnpackMM(mService) * 1000;
	}

}
