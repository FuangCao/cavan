package com.cavan.huahardwareinfo;

import android.os.Build;

public class HuaLcdVendorInfo {
	private static final HuaLcdVendorInfo[] mVendorInfo_HS8801 = {
		new HuaLcdVendorInfo(0x8012, 0, R.string.vendor_name_lide),
		new HuaLcdVendorInfo(0x8012, 1, R.string.vendor_name_boyi),
		new HuaLcdVendorInfo(0x9806, 0, R.string.vendor_name_tongxinda)
	};

	private int mId;
	private int mVendorId;
	private int mVendorName;

	public HuaLcdVendorInfo(int id, int vendorId, int vendorName) {
		super();
		mId = id;
		mVendorId = vendorId;
		mVendorName = vendorName;
	}

	public int getId() {
		return mId;
	}

	public int getVendorId() {
		return mVendorId;
	}

	public int getVendorName() {
		return mVendorName;
	}

	public static HuaLcdVendorInfo getLcdVendorInfo(int id, int vendorId) {
		HuaLcdVendorInfo[] vendorInfos;

		if (Build.BOARD.equals("hs8801")) {
			vendorInfos = mVendorInfo_HS8801;
		} else {
			return null;
		}

		for (HuaLcdVendorInfo vendorInfo : vendorInfos) {
			if (vendorInfo.getId() == id && vendorInfo.getVendorId() == vendorId) {
				return vendorInfo;
			}
		}

		return null;
	}
}