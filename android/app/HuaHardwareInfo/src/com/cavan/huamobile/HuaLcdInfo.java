package com.cavan.huamobile;

import java.util.HashMap;
import java.util.Map.Entry;

public class HuaLcdInfo {
	private static HashMap<Integer, String> mHashMapIdIc = new HashMap<Integer, String>();
	private static HashMap<String, Integer> mHashMapNameId = new HashMap<String, Integer>();

	static {
		mHashMapIdIc.put(0x8009, "OTM8009");
		mHashMapIdIc.put(0x8012, "OTM8012A");
		mHashMapIdIc.put(0x8018B, "OTM8018B");
		mHashMapIdIc.put(0x8019A, "OTM8019A");

		mHashMapIdIc.put(0x9488, "ILI9488");
		mHashMapIdIc.put(0x9805, "ILI9805");
		mHashMapIdIc.put(0x9806, "ILI9806");
		mHashMapIdIc.put(0x9806E, "ILI9806E");
		mHashMapIdIc.put(0x9826, "ILI9806H");

		mHashMapIdIc.put(0x5512, "NT35512");
		mHashMapIdIc.put(0x5521, "NT35521");
		mHashMapIdIc.put(0x5590, "NT35590");
		mHashMapIdIc.put(0x5596, "NT35596");

		mHashMapIdIc.put(0x8379A, "HX8379A");
		mHashMapIdIc.put(0x8389B, "HX8389B");
		mHashMapIdIc.put(0x8394A, "HX8394A");

		mHashMapIdIc.put(0x1868, "RM68180");
		mHashMapIdIc.put(0x5510, "CD35510");

		mHashMapNameId.put("qcom,mdss_dsi_ili9488_hvga_video", 0x9488);
		mHashMapNameId.put("qcom,mdss_dsi_ili9806e_hwvga_video", 0x9806E);
		mHashMapNameId.put("qcom,mdss_dsi_ili9806e_fwvga_video", 0x9806E);
		mHashMapNameId.put("qcom,mdss_dsi_otm8018b_fwvga_video", 0x8018B);
		mHashMapNameId.put("qcom,mdss_dsi_otm8018b_hwvga_video", 0x8018B);
		mHashMapNameId.put("qcom,mdss_dsi_hx8379a_wvga_video", 0x8379A);
		mHashMapNameId.put("qcom,mdss_dsi_nt35512_hwvga_video", 0x5512);
	}

	private int mId;
	private String mName;
	private String mShortName;
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
		mName = name.toLowerCase();

		String fields[] = mName.split("\\s*,\\s*");
		if (fields != null && fields.length > 2) {
			mVendorId = -1;
			mShortName = fields[2];
		} else {
			mVendorId = 0;
			mShortName = null;
		}

		Integer id = mHashMapNameId.get(mName);
		if (id == null) {
			for (Entry<Integer, String> entry: mHashMapIdIc.entrySet()) {
				if (mName.contains(entry.getValue().toLowerCase())) {
					id = entry.getKey();
					break;
				}
			}
		}

		mId = id == null ? 0 : id;

		fillVendorInfo();
	}

	private void fillVendorInfo() {
		mIc = mHashMapIdIc.get(mId);
		if (mShortName == null) {
			mVendorInfo = HuaLcdVendorInfo.getLcdVendorInfo(mId, mVendorId);
		} else {
			mVendorInfo = HuaLcdVendorInfo.getLcdVendorInfo(mId, mShortName);
		}
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