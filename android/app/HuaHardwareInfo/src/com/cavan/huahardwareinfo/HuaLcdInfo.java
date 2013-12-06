package com.cavan.huahardwareinfo;

import java.util.HashMap;

public class HuaLcdInfo {
	private static HashMap<Integer, String> mHashMapIdIc = new HashMap<Integer, String>();

	static {
		mHashMapIdIc.put(0x5510, "CD35510");
		mHashMapIdIc.put(0x8012, "OTM8012A");
		mHashMapIdIc.put(0x1868, "RM68180");
		mHashMapIdIc.put(0x9805, "ILI9805");
		mHashMapIdIc.put(0x8009, "OTM8009");
		mHashMapIdIc.put(0x9806, "ILI9806");
		mHashMapIdIc.put(0x9826, "ILI9806H");
	}

	private int mId;
	private int mVendorId;
	private String mIc;
	private HuaLcdVendorInfo mVendorInfo;

	public HuaLcdInfo(int id, int vendorId) {
		super();
		mId = id;
		mVendorId = vendorId;
		mIc = mHashMapIdIc.get(id);
		mVendorInfo = HuaLcdVendorInfo.getLcdVendorInfo(id, vendorId);
	}

	public int getId() {
		return mId;
	}

	public int getVendorId() {
		return mVendorId;
	}

	public String getIc() {
		return mIc;
	}

	public HuaLcdVendorInfo getVendorInfo() {
		return mVendorInfo;
	}
}