package com.cavan.huahardwareinfo;

import com.cavan.huahardwareinfo.R;

import android.os.Build;

public class HuaLcdVendorInfo {
	private static final HuaLcdVendorInfo[] mVendorInfo_HS8801 = {
		new HuaLcdVendorInfo(0x8012, 0, R.string.vendor_name_lide),
		new HuaLcdVendorInfo(0x8012, 1, R.string.vendor_name_boyi),
		new HuaLcdVendorInfo(0x8009, 1, R.string.vendor_name_apex),
		new HuaLcdVendorInfo(0x9826, 0, R.string.vendor_name_tongxingda),
		new HuaLcdVendorInfo(0x9806, 0, R.string.vendor_name_tongxingda)
	};

	private static final HuaLcdVendorInfo[] mVendorInfo_ZC2501 = {
		new HuaLcdVendorInfo(0x9806E, 0, R.string.vendor_name_tongxingda),
		new HuaLcdVendorInfo(0x9806E, R.string.vendor_name_tongxingda, "txd"),
		new HuaLcdVendorInfo(0x9806E, R.string.vendor_name_yashi, "yashi"),
		new HuaLcdVendorInfo(0x8018B, 0, R.string.vendor_name_zhongxianwei),
		new HuaLcdVendorInfo(0x8018B, R.string.vendor_name_zhongxianwei, "cdt"),
		new HuaLcdVendorInfo(0x8019A, 0, R.string.vendor_name_huayu),
		new HuaLcdVendorInfo(0x5512, 0, R.string.vendor_name_lide),
		new HuaLcdVendorInfo(0x5512, R.string.vendor_name_lide, "lide")
	};

	private int mId;
	private int mVendorId;
	private int mVendorName;
	private String mShortName;

	public HuaLcdVendorInfo(int id, int vendorId, int vendorName, String shortName) {
		super();
		mId = id;
		mVendorId = vendorId;
		mVendorName = vendorName;
		mShortName = shortName;
	}

	public HuaLcdVendorInfo(int id, int vendorId, int vendorName) {
		this(id, vendorId, vendorName, null);
	}

	public HuaLcdVendorInfo(int id, int vendorName, String shortName) {
		this(id, -1, vendorName, shortName);
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

	public static HuaLcdVendorInfo[] getVendorInfoList() {
		if (Build.BOARD.equals("hs8801")) {
			return mVendorInfo_HS8801;
		} else if (Build.DEVICE.equals("P810N30") || Build.DEVICE.equals("APT_TW_P810N30")) {
			return mVendorInfo_ZC2501;
		} else {
			return null;
		}
	}

	public static HuaLcdVendorInfo getLcdVendorInfo(int id, int vendorId) {
		HuaLcdVendorInfo[] vendorInfos = getVendorInfoList();
		if (vendorInfos == null) {
			return null;
		}

		for (HuaLcdVendorInfo vendorInfo : vendorInfos) {
			if (vendorInfo.getId() == id && vendorInfo.getVendorId() == vendorId) {
				return vendorInfo;
			}
		}

		return null;
	}

	public static HuaLcdVendorInfo getLcdVendorInfo(int id, String shortName) {
		HuaLcdVendorInfo[] vendorInfos = getVendorInfoList();
		if (vendorInfos == null) {
			return null;
		}

		for (HuaLcdVendorInfo vendorInfo : vendorInfos) {
			if (vendorInfo.getId() == id && shortName.equals(vendorInfo.mShortName)) {
				return vendorInfo;
			}
		}

		return null;
	}
}
