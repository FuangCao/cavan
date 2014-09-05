package com.cavan.huamobile;

import com.cavan.huahardwareinfo.R;

import android.os.Build;

public class HuaCameraInfo {
	private static final HuaCameraInfo[] mCameraInfo_HS8801 = {
		new HuaCameraInfo("AT2250", R.string.vendor_name_hongtu, R.string.vendor_name_sanglaishi),
		new HuaCameraInfo("SP2518", R.string.vendor_name_sibike, R.string.vendor_name_boyi)
	};
	private static final HuaCameraInfo[] mCameraInfo_ZC2501 = {
		new HuaCameraInfo("GC2035", R.string.vendor_name_geke, R.string.vendor_name_tongju),
		new HuaCameraInfo("GC0339", R.string.vendor_name_geke, R.string.vendor_name_boyi),
		new HuaCameraInfo("GC5004", R.string.vendor_name_geke, R.string.vendor_name_geke),
		new HuaCameraInfo("OV5648", R.string.vendor_name_ov, R.string.vendor_name_ov),
		new HuaCameraInfo("SP2529", R.string.vendor_name_sibike, R.string.vendor_name_boyi)
	};

	private String mName;
	private int mIcVendor;
	private int mVendorName;

	public HuaCameraInfo(String name, int icVendor, int vendorName) {
		super();
		mName = name;
		mIcVendor = icVendor;
		mVendorName = vendorName;
	}

	public String getName() {
		return mName;
	}

	public int getIcVendor() {
		return mIcVendor;
	}

	public int getVendorName() {
		return mVendorName;
	}

	public static HuaCameraInfo getCameraInfo(String name) {
		HuaCameraInfo[] infos;

		if (Build.BOARD.equals("hs8801")) {
			infos = mCameraInfo_HS8801;
		} else if (Build.DEVICE.equals("P810N30") || Build.DEVICE.equals("APT_TW_P810N30")) {
			infos = mCameraInfo_ZC2501;
		} else {
			return null;
		}

		name = name.toUpperCase();

		for (HuaCameraInfo info : infos) {
			if (name.contains(info.getName())) {
				return info;
			}
		}

		return null;
	}
}