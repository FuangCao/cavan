package com.cavan.huahardwareinfo;

public class HuaFlashInfo {
	private static final HuaFlashInfo[] mHuaFlashInfoList = {
		new HuaFlashInfo("AD:BC:90:55:54", "H9DA4GH2GJAMCR-4EM", "512M(Flash) + 256M(DDR)", R.string.vendor_name_hynix),
		new HuaFlashInfo("EC:BC:00:55:54", "K524G2GACI-B050", "512M(Flash) + 256M(DDR)", R.string.vendor_name_samsung)
	};

	private String mId;
	private String mIc;
	private String mCapacity;
	private int mVendorName;

	public HuaFlashInfo(String id, String ic, String capacity, int vendorName) {
		super();
		mId = id;
		mIc = ic;
		mCapacity = capacity;
		mVendorName = vendorName;
	}

	public String getId() {
		return mId;
	}

	public String getIc() {
		return mIc;
	}

	public String getCapacity() {
		return mCapacity;
	}

	public int getVendorName() {
		return mVendorName;
	}

	public static HuaFlashInfo getFlashInfo(String id) {
		id = id.toUpperCase();

		for (HuaFlashInfo info : mHuaFlashInfoList) {
			if (info.getId().equals(id)) {
				return info;
			}
		}

		return null;
	}
}
