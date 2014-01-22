package com.cavan.huahardwareinfo;

import java.util.HashMap;

import android.os.Build;

public class HuaTouchscreenVendorInfo {
	private static final String TAG = "Cavan";
	private static HashMap<Integer, String> mHashMapShortName = new HashMap<Integer, String>();
	private static HashMap<Integer, Integer> mHashMapVendor = new HashMap<Integer,Integer>();

	private int mVendorId;
	private int mFwVersion;
	private int mVendorName;
	private String mShortName;

	static {
		mHashMapShortName.put(R.string.vendor_name_dianjing, "dianjing");
		mHashMapShortName.put(R.string.vendor_name_lianchuang, "lianchuang");
		mHashMapShortName.put(R.string.vendor_name_lihaojie, "lihaojie");
		mHashMapShortName.put(R.string.vendor_name_simcom, "simcom");
		mHashMapShortName.put(R.string.vendor_name_tongxincheng, "tongxincheng");
		mHashMapShortName.put(R.string.vendor_name_yeji, "yeji");
		mHashMapShortName.put(R.string.vendor_name_yingmao, "yingmao");
		mHashMapShortName.put(R.string.vendor_name_yuansheng, "yuansheng");
		mHashMapShortName.put(R.string.vendor_name_lihe, "lihe");

		if (Build.BOARD.equals("h1")) {
			mHashMapVendor.put(0x13, R.string.vendor_name_dianjing);
			mHashMapVendor.put(0x31, R.string.vendor_name_yeji);
			mHashMapVendor.put(0x30, R.string.vendor_name_simcom);
		} else if (Build.BOARD.equals("h2")) {
			mHashMapVendor.put(0x94, R.string.vendor_name_yingmao);
			mHashMapVendor.put(0x87, R.string.vendor_name_lianchuang);
			mHashMapVendor.put(0x11, R.string.vendor_name_lihaojie);
		} else if (Build.BOARD.equals("h3")) {
			mHashMapVendor = new HashMap<Integer, Integer>();
			mHashMapVendor.put(0xdb, R.string.vendor_name_dianjing);
			mHashMapVendor.put(0xda, R.string.vendor_name_tongxincheng);
			mHashMapVendor.put(0x30, R.string.vendor_name_simcom);
		} else if (Build.BOARD.equals("h4")) {
			mHashMapVendor.put(0xba, R.string.vendor_name_dianjing);
			mHashMapVendor.put(0x11, R.string.vendor_name_lihaojie);
			mHashMapVendor.put(0x74, R.string.vendor_name_yuansheng);
			mHashMapVendor.put(0x9b, R.string.vendor_name_yuansheng);
			mHashMapVendor.put(0x94, R.string.vendor_name_yingmao);
		} else if (Build.BOARD.equals("h5")) {
			mHashMapVendor.put(0x32, R.string.vendor_name_lianchuang);
			mHashMapVendor.put(0x13, R.string.vendor_name_dianjing);
			mHashMapVendor.put(0x30, R.string.vendor_name_simcom);
		} else if (Build.BOARD.equals("hs8801")) {
			mHashMapVendor.put(0x70, R.string.vendor_name_lihe);
			mHashMapVendor.put(0x58, R.string.vendor_name_lihe);
			mHashMapVendor.put(0x59, R.string.vendor_name_dianjing);
			mHashMapVendor.put(0x32, R.string.vendor_name_lianchuang);
			mHashMapVendor.put(0x18, R.string.vendor_name_yuda);
			mHashMapVendor.put(0xdb, R.string.vendor_name_oufei);
			mHashMapVendor.put(0x19, R.string.vendor_name_yeji);
			mHashMapVendor.put(0x20, R.string.vendor_name_junda);
		} else if (Build.BOARD.equals("cp8076")) {
			mHashMapVendor.put(0x76, R.string.vendor_name_yingmao);
			mHashMapVendor.put(0x66, R.string.vendor_name_yingmao);
			mHashMapVendor.put(0x80, R.string.vendor_name_yeji);
			mHashMapVendor.put(0x79, R.string.vendor_name_chuangshi);
			mHashMapVendor.put(0x78, R.string.vendor_name_chuangshi);
			mHashMapVendor.put(0x77, R.string.vendor_name_dianmian);
			mHashMapVendor.put(0x81, R.string.vendor_name_junda);
			mHashMapVendor.put(0x82, R.string.vendor_name_dianjing);
			mHashMapVendor.put(0x83, R.string.vendor_name_lihe);
		}
	}

	public HuaTouchscreenVendorInfo(int fwId) {
		super();

		mVendorId = (fwId >> 8) & 0xFF;
		mFwVersion = fwId & 0xFF;

		Integer vendorName = mHashMapVendor.get(mVendorId);
		if (vendorName == null) {
			mVendorName = R.string.vendor_name_unknown;
		} else {
			mVendorName = vendorName;
		}

		mShortName = mHashMapShortName.get(mVendorName);
		if (mShortName == null) {
			mShortName = "xxx";
		}
	}

	public int getVendorId() {
		return mVendorId;
	}

	public int getFwVersion() {
		return mFwVersion;
	}

	public int getVendorName() {
		return mVendorName;
	}

	public String getShortName() {
		return mShortName;
	}
}
