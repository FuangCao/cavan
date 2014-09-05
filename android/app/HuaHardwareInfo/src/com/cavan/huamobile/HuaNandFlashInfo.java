package com.cavan.huamobile;

import java.util.HashMap;

import com.cavan.huahardwareinfo.R;

class NandFlashDevice {
	private String name;
	int mPageSize;
	int mChipSize;
	int mEraseSize;

	public NandFlashDevice(String name, int pageSize, int chipSize, int eraseSize) {
		super();
		this.name = name;
		this.mPageSize = pageSize;
		this.mChipSize = chipSize;
		this.mEraseSize = eraseSize;
	}

	private String sizeToString(int size) {
		if (size > 1024) {
			return Integer.toString(size / 1024) + "GiB";
		}

		return Integer.toString(size) + "MiB";
	}

	public String getName() {
		return name;
	}

	public int getPageSize() {
		return mPageSize;
	}

	public int getChipSize() {
		return mChipSize;
	}

	public String getChipSizeString() {
		return sizeToString(mChipSize);
	}

	public int getEraseSize() {
		return mEraseSize;
	}
};

public class HuaNandFlashInfo {
	private static HashMap<Integer, Integer> mHashMapVendor = new HashMap<Integer, Integer>();
	private static HashMap<Integer, NandFlashDevice> mHashMapNandDevice = new HashMap<Integer, NandFlashDevice>();
	private static HashMap<String, String> mHashMapChipName = new HashMap<String, String>();

	static {
		mHashMapVendor.put(0x98, R.string.vendor_name_toshiba);
		mHashMapVendor.put(0xec, R.string.vendor_name_samsung);
		mHashMapVendor.put(0x04, R.string.vendor_name_fujitsu);
		mHashMapVendor.put(0x8f, R.string.vendor_name_national);
		mHashMapVendor.put(0x07, R.string.vendor_name_renesas);
		mHashMapVendor.put(0x20, R.string.vendor_name_st_micro);
		mHashMapVendor.put(0xad, R.string.vendor_name_hynix);
		mHashMapVendor.put(0x2c, R.string.vendor_name_micron);
		mHashMapVendor.put(0x01, R.string.vendor_name_amd);
		mHashMapVendor.put(0x20, R.string.vendor_name_st_micro);

		mHashMapNandDevice.put(0x6e, new NandFlashDevice("NAND 1MiB 5V 8-bit", 256, 1, 0x1000));
		mHashMapNandDevice.put(0x64, new NandFlashDevice("NAND 2MiB 5V 8-bit", 0x64, 2, 0x1000));
		mHashMapNandDevice.put(0x6b, new NandFlashDevice("NAND 4MiB 5V 8-bit", 0x6b, 4, 0x2000));
		mHashMapNandDevice.put(0xe8, new NandFlashDevice("NAND 1MiB 3.3V 8-bit", 0xe8, 1, 0x1000));
		mHashMapNandDevice.put(0xec, new NandFlashDevice("NAND 1MiB 3.3V 8-bit", 0xec, 1, 0x1000));
		mHashMapNandDevice.put(0xea, new NandFlashDevice("NAND 2MiB 3.3V 8-bit", 0xea, 2, 0x1000));
		mHashMapNandDevice.put(0xd5, new NandFlashDevice("NAND 4MiB 3.3V 8-bit", 0xd5, 4, 0x2000));
		mHashMapNandDevice.put(0xe3, new NandFlashDevice("NAND 4MiB 3.3V 8-bit", 0xe3, 4, 0x2000));
		mHashMapNandDevice.put(0xe5, new NandFlashDevice("NAND 4MiB 3.3V 8-bit", 0xe5, 4, 0x2000));
		mHashMapNandDevice.put(0xd6, new NandFlashDevice("NAND 8MiB 3.3V 8-bit", 0xd6, 8, 0x2000));
		mHashMapNandDevice.put(0x39, new NandFlashDevice("NAND 8MiB 1.8V 8-bit", 0x39, 8, 0x2000));
		mHashMapNandDevice.put(0xe6, new NandFlashDevice("NAND 8MiB 3.3V 8-bit", 0xe6, 8, 0x2000));
		mHashMapNandDevice.put(0x49, new NandFlashDevice("NAND 8MiB 1.8V 16-bit", 0x49, 8, 0x2000));
		mHashMapNandDevice.put(0x59, new NandFlashDevice("NAND 8MiB 3.3V 16-bit", 0x59, 8, 0x2000));
		mHashMapNandDevice.put(0x33, new NandFlashDevice("NAND 16MiB 1.8V 8-bit", 0x33, 16, 0x4000));
		mHashMapNandDevice.put(0x73, new NandFlashDevice("NAND 16MiB 3.3V 8-bit", 0x73, 16, 0x4000));
		mHashMapNandDevice.put(0x43, new NandFlashDevice("NAND 16MiB 1.8V 16-bit", 0x43, 16, 0x4000));
		mHashMapNandDevice.put(0x53, new NandFlashDevice("NAND 16MiB 3.3V 16-bit", 0x53, 16, 0x4000));
		mHashMapNandDevice.put(0x35, new NandFlashDevice("NAND 32MiB 1.8V 8-bit", 0x35, 32, 0x4000));
		mHashMapNandDevice.put(0x75, new NandFlashDevice("NAND 32MiB 3.3V 8-bit", 0x75, 32, 0x4000));
		mHashMapNandDevice.put(0x45, new NandFlashDevice("NAND 32MiB 1.8V 16-bit", 0x45, 32, 0x4000));
		mHashMapNandDevice.put(0x55, new NandFlashDevice("NAND 32MiB 3.3V 16-bit", 0x55, 32, 0x4000));
		mHashMapNandDevice.put(0x36, new NandFlashDevice("NAND 64MiB 1.8V 8-bit", 0x36, 64, 0x4000));
		mHashMapNandDevice.put(0x76, new NandFlashDevice("NAND 64MiB 3.3V 8-bit", 0x76, 64, 0x4000));
		mHashMapNandDevice.put(0x46, new NandFlashDevice("NAND 64MiB 1.8V 16-bit", 0x46, 64, 0x4000));
		mHashMapNandDevice.put(0x56, new NandFlashDevice("NAND 64MiB 3.3V 16-bit", 0x56, 64, 0x4000));
		mHashMapNandDevice.put(0x78, new NandFlashDevice("NAND 128MiB 1.8V 8-bit", 0x78, 128, 0x4000));
		mHashMapNandDevice.put(0x39, new NandFlashDevice("NAND 128MiB 1.8V 8-bit", 0x39, 128, 0x4000));
		mHashMapNandDevice.put(0x79, new NandFlashDevice("NAND 128MiB 3.3V 8-bit", 0x79, 128, 0x4000));
		mHashMapNandDevice.put(0x72, new NandFlashDevice("NAND 128MiB 1.8V 16-bit", 0x72, 128, 0x4000));
		mHashMapNandDevice.put(0x49, new NandFlashDevice("NAND 128MiB 1.8V 16-bit", 0x49, 128, 0x4000));
		mHashMapNandDevice.put(0x74, new NandFlashDevice("NAND 128MiB 3.3V 16-bit", 0x74, 128, 0x4000));
		mHashMapNandDevice.put(0x59, new NandFlashDevice("NAND 128MiB 3.3V 16-bit", 0x59, 128, 0x4000));
		mHashMapNandDevice.put(0x71, new NandFlashDevice("NAND 256MiB 3.3V 8-bit", 0x71, 256, 0x4000));
		mHashMapNandDevice.put(0xA2, new NandFlashDevice("NAND 64MiB 1.8V 8-bit", 0xA2, 64, 0));
		mHashMapNandDevice.put(0xF2, new NandFlashDevice("NAND 64MiB 3.3V 8-bit", 0xF2, 64, 0));
		mHashMapNandDevice.put(0xB2, new NandFlashDevice("NAND 64MiB 1.8V 16-bit", 0xB2, 64, 0));
		mHashMapNandDevice.put(0xC2, new NandFlashDevice("NAND 64MiB 3.3V 16-bit", 0xC2, 64, 0));
		mHashMapNandDevice.put(0xA1, new NandFlashDevice("NAND 128MiB 1.8V 8-bit", 0xA1, 128, 0));
		mHashMapNandDevice.put(0xF1, new NandFlashDevice("NAND 128MiB 3.3V 8-bit", 0xF1, 128, 0));
		mHashMapNandDevice.put(0xD1, new NandFlashDevice("NAND 128MiB 3.3V 8-bit", 0xD1, 128, 0));
		mHashMapNandDevice.put(0xB1, new NandFlashDevice("NAND 128MiB 1.8V 16-bit", 0xB1, 128, 0));
		mHashMapNandDevice.put(0xC1, new NandFlashDevice("NAND 128MiB 3.3V 16-bit", 0xC1, 128, 0));
		mHashMapNandDevice.put(0xAA, new NandFlashDevice("NAND 256MiB 1.8V 8-bit", 0xAA, 256, 0));
		mHashMapNandDevice.put(0xDA, new NandFlashDevice("NAND 256MiB 3.3V 8-bit", 0xDA, 256, 0));
		mHashMapNandDevice.put(0xBA, new NandFlashDevice("NAND 256MiB 1.8V 16-bit", 0xBA, 256, 0));
		mHashMapNandDevice.put(0xCA, new NandFlashDevice("NAND 256MiB 3.3V 16-bit", 0xCA, 256, 0));
		mHashMapNandDevice.put(0xAC, new NandFlashDevice("NAND 512MiB 1.8V 8-bit", 0xAC, 512, 0));
		mHashMapNandDevice.put(0xDC, new NandFlashDevice("NAND 512MiB 3.3V 8-bit", 0xDC, 512, 0));
		mHashMapNandDevice.put(0xBC, new NandFlashDevice("NAND 512MiB 1.8V 16-bit", 0xBC, 512, 0));
		mHashMapNandDevice.put(0xCC, new NandFlashDevice("NAND 512MiB 3.3V 16-bit", 0xCC, 512, 0));
		mHashMapNandDevice.put(0xA3, new NandFlashDevice("NAND 1GiB 1.8V 8-bit", 0xA3, 1024, 0));
		mHashMapNandDevice.put(0xD3, new NandFlashDevice("NAND 1GiB 3.3V 8-bit", 0xD3, 1024, 0));
		mHashMapNandDevice.put(0xB3, new NandFlashDevice("NAND 1GiB 1.8V 16-bit", 0xB3, 1024, 0));
		mHashMapNandDevice.put(0xC3, new NandFlashDevice("NAND 1GiB 3.3V 16-bit", 0xC3, 1024, 0));
		mHashMapNandDevice.put(0xA5, new NandFlashDevice("NAND 2GiB 1.8V 8-bit", 0xA5, 2048, 0));
		mHashMapNandDevice.put(0xD5, new NandFlashDevice("NAND 2GiB 3.3V 8-bit", 0xD5, 2048, 0));
		mHashMapNandDevice.put(0xB5, new NandFlashDevice("NAND 2GiB 1.8V 16-bit", 0xB5, 2048, 0));
		mHashMapNandDevice.put(0xC5, new NandFlashDevice("NAND 2GiB 3.3V 16-bit", 0xC5, 2048, 0));
		mHashMapNandDevice.put(0x01, new NandFlashDevice("AND 128MiB 3.3V 8-bit", 0x01, 128, 0x4000));

		mHashMapChipName.put("ad:bc:90:55:54", "H9DA4GH2GJAMCR-4EM");
		mHashMapChipName.put("ec:bc:00:55:54", "K524G2GACI-B050");
		mHashMapChipName.put("2c:b3:d1:55:5a", "MT29C8G96MAAFBACKD-5");
		mHashMapChipName.put("2c:ba:80:55:50", "MT29C2G48MAKLCJA-5");
		mHashMapChipName.put("2c:bc:90:55:56", "KTR0405AS-HHg1");
		mHashMapChipName.put("98:ac:90:15:76", "TYBC0A111392KC");
		mHashMapChipName.put("98:bc:90:55:76", "TYBC0A111430KC");
		mHashMapChipName.put("98:bc:90:66:76", "KSLCCBL1FB2G3A");
		mHashMapChipName.put("ad:bc:90:11:00", "H9DA4VH4JJMMCR-4EMi");
		mHashMapChipName.put("ad:bc:90:55:56", "H9DA4GH2GJBMCR-4EM");
		mHashMapChipName.put("ec:b3:01:66:5a", "KBY00U00VA-B450");
		mHashMapChipName.put("ec:bc:00:55:54", "K524G2GACI-B050");
		mHashMapChipName.put("ec:bc:00:6a:56", "K524G2GACH-B050");
		mHashMapChipName.put("ec:bc:01:55:48", "KBY00N00HM-A448");
	}

	private String mId;
	private String mIc;
	private int mVendor;
	private NandFlashDevice mFlashDevice;

	public HuaNandFlashInfo(String id, int vendor) {
		super();
		mVendor = vendor;
	}

	public String getId() {
		return mId;
	}

	public String getIc() {
		return mIc;
	}

	public int getVendor() {
		return mVendor;
	}

	public NandFlashDevice getFlashDevice() {
		return mFlashDevice;
	}

	public void setIc(String ic) {
		mIc = ic;
	}

	public void setFlashDevice(NandFlashDevice flashDevice) {
		mFlashDevice = flashDevice;
	}

	public static HuaNandFlashInfo getFlashInfo(String id) {
		String[] ids = id.split(":");
		if (ids == null || ids.length != 5) {
			return null;
		}

		Integer vendor = mHashMapVendor.get(Integer.parseInt(ids[0], 16));
		if (vendor == null) {
			return null;
		}

		HuaNandFlashInfo info = new HuaNandFlashInfo(id, vendor);
		if (info == null) {
			return null;
		}

		info.setIc(mHashMapChipName.get(id.toLowerCase()));
		info.setFlashDevice(mHashMapNandDevice.get(Integer.parseInt(ids[1], 16)));

		return info;
	}
}
