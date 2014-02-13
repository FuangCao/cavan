package com.cavan.huahardwareinfo;

import java.util.HashMap;

public class HuaMmcInfo {
	static private HashMap<Integer, Integer> mHashMapVendorName = new HashMap<Integer, Integer>();

	static {
		mHashMapVendorName.put(0x02, R.string.vendor_name_sandisk);
		mHashMapVendorName.put(0x11, R.string.vendor_name_toshiba);
		mHashMapVendorName.put(0x13, R.string.vendor_name_micron);
		mHashMapVendorName.put(0x15, R.string.vendor_name_samsung);
		mHashMapVendorName.put(0x90, R.string.vendor_name_hynix);
	}

	static Integer getVendorNameById(int id) {
		return mHashMapVendorName.get(id);
	}
}
