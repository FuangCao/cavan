package com.cavan.accessibility;

import com.cavan.android.CavanPackageName;

public class CavanAccessibilityAlipay extends CavanAccessibilityPackage {

	private static final String[] BACK_VIEW_IDS = {
		"com.alipay.mobile.ui:id/title_bar_back_button",
		"com.alipay.android.phone.discovery.envelope:id/coupon_chai_close",
		"com.alipay.mobile.nebula:id/h5_tv_nav_back",
	};

	private static final String[] OVER_VIEW_IDS = {
		"com.alipay.android.phone.discovery.envelope:id/coupon_action",
		"com.alipay.android.phone.discovery.envelope:id/sns_coupon_detail_action",
	};

	public static CavanAccessibilityAlipay instance;

	private CavanRedPacketAlipay mCode;

	public CavanAccessibilityAlipay(CavanAccessibilityService service, String name) {
		super(service, CavanPackageName.ALIPAY);
		instance = this;
	}

	public boolean addPacket(String code) {
		return addPacket(new CavanRedPacketAlipay(code));
	}

	@Override
	protected void initWindows() {
		super.initWindows();
	}

}
