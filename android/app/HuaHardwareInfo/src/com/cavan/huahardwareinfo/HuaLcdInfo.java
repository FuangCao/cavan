package com.cavan.huahardwareinfo;

import java.util.HashMap;

public class HuaLcdInfo {
	private static HashMap<Integer, String> mHashMapIdIc = new HashMap<Integer, String>();
	private static HashMap<String, Integer> mHashMapNameId = new HashMap<String, Integer>();

	static {
		mHashMapIdIc.put(0x5510, "CD35510");
		mHashMapIdIc.put(0x8012, "OTM8012A");
		mHashMapIdIc.put(0x1868, "RM68180");
		mHashMapIdIc.put(0x9805, "ILI9805");
		mHashMapIdIc.put(0x8009, "OTM8009");
		mHashMapIdIc.put(0x9806, "ILI9806");
		mHashMapIdIc.put(0x9816, "ILI9806E");
		mHashMapIdIc.put(0x9826, "ILI9806H");
		mHashMapIdIc.put(0x5512, "NT35512");

		mHashMapNameId.put("qcom,mdss_dsi_ili9806e_hwvga_video", 0x9816);
		mHashMapNameId.put("qcom,mdss_dsi_nt35512_hwvga_video", 0x5512);
	}

	private int mId;
	private String mName;
	private int mVendorId;
	private String mIc;
	private HuaLcdVendorInfo mVendorInfo;

	public HuaLcdInfo(int id, int vendorId) {
		super();
		mId = id;
		mVendorId = vendorId;
		fillVendorInfo();
	}

	public HuaLcdInfo(String name) {
		super();
		mName = name;
		mVendorId = 0;
		mId = mHashMapNameId.get(mName);
		fillVendorInfo();
	}

	private void fillVendorInfo() {
		mIc = mHashMapIdIc.get(mId);
		mVendorInfo = HuaLcdVendorInfo.getLcdVendorInfo(mId, mVendorId);
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