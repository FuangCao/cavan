package com.cavan.touchscreen;

import android.os.Build;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

class VendorShortNameMap {
	private int mResourceId;
	private String mShortName;

	public VendorShortNameMap(int id, String name) {
		mResourceId = id;
		mShortName = name;
	}

	public int getResourceId() {
		return mResourceId;
	}

	public String getShortName() {
		return mShortName;
	}

	public static VendorShortNameMap findMapByShortName(VendorShortNameMap[] maps, String name) {
		for (VendorShortNameMap map : maps) {
			if (map.getShortName().equals(name))
				return map;
		}

		return null;
	}

	public static VendorShortNameMap findMapByResource(VendorShortNameMap[] maps, int id) {
		for (VendorShortNameMap map : maps) {
			if (map.getResourceId() == id)
				return map;
		}

		return null;
	}
}

class VendorNameMap {
	private int mVendorId;
	private int mResourceId;

	public VendorNameMap(int id, int resource) {
		mVendorId = id;
		mResourceId = resource;
	}

	public int getVendorId() {
		return mVendorId;
	}

	public int getResourceId() {
		return mResourceId;
	}

	public static VendorNameMap findMapByVendorId(VendorNameMap[] maps, int id) {
		for (VendorNameMap map : maps) {
			if (map.getVendorId() == id) {
				return map;
			}
		}

		return null;
	}

	public static VendorNameMap findMapByResource(VendorNameMap[] maps, int resource) {
		for (VendorNameMap map : maps) {
			if (map.getResourceId() == resource) {
				return map;
			}
		}

		return null;
	}
}

class VendorMap {
	private String mProjectName;
	private VendorNameMap []mNameMaps;

	public VendorMap(String project, VendorNameMap[] nameMaps) {
		mProjectName = project;
		mNameMaps = nameMaps;
	}

	public String getProjectName() {
		return mProjectName;
	}

	public VendorNameMap[] getNameMaps() {
		return mNameMaps;
	}

	public int findResourceByVendorId(int id) {
		VendorNameMap map = VendorNameMap.findMapByVendorId(mNameMaps, id);
		if (map == null) {
			return R.string.vendor_name_unknown;
		}

		return map.getResourceId();
	}

	public static VendorMap findMapByProjectName(VendorMap[] maps, String project) {
		for (VendorMap map : maps) {
			if (map.getProjectName().equals(project)) {
				return map;
			}
		}

		return null;
	}
}

public class DeviceID implements Parcelable {
	private static final String TAG = "Cavan";

	private VendorShortNameMap []mShortNameMaps = {
		new VendorShortNameMap(R.string.vendor_name_dianjing, "dianjing"),
		new VendorShortNameMap(R.string.vendor_name_lianchuang, "lianchuang"),
		new VendorShortNameMap(R.string.vendor_name_lihaojie, "lihaojie"),
		new VendorShortNameMap(R.string.vendor_name_simcom, "simcom"),
		new VendorShortNameMap(R.string.vendor_name_tongxincheng, "tongxincheng"),
		new VendorShortNameMap(R.string.vendor_name_yeji, "yeji"),
		new VendorShortNameMap(R.string.vendor_name_yingmao, "yingmao"),
		new VendorShortNameMap(R.string.vendor_name_yuansheng, "yuansheng"),
		new VendorShortNameMap(R.string.vendor_name_lihe, "lihe"),
	};

	private VendorMap[] mVendorMaps = {
		new VendorMap("h1", new VendorNameMap[] {
			new VendorNameMap(0x13, R.string.vendor_name_dianjing),
			new VendorNameMap(0x31, R.string.vendor_name_yeji),
			new VendorNameMap(0x30, R.string.vendor_name_simcom),
		}),
		new VendorMap("h2", new VendorNameMap[] {
			new VendorNameMap(0x94, R.string.vendor_name_yingmao),
			new VendorNameMap(0x87, R.string.vendor_name_lianchuang),
			new VendorNameMap(0x11, R.string.vendor_name_lihaojie),
		}),
		new VendorMap("h3", new VendorNameMap[] {
			new VendorNameMap(0xdb, R.string.vendor_name_dianjing),
			new VendorNameMap(0xda, R.string.vendor_name_tongxincheng),
			new VendorNameMap(0x30, R.string.vendor_name_simcom),
		}),
		new VendorMap("h4", new VendorNameMap[] {
			new VendorNameMap(0xba, R.string.vendor_name_dianjing),
			new VendorNameMap(0x11, R.string.vendor_name_lihaojie),
			new VendorNameMap(0x74, R.string.vendor_name_yuansheng),
		}),
		new VendorMap("h5", new VendorNameMap[] {
			new VendorNameMap(0x32, R.string.vendor_name_lianchuang),
			new VendorNameMap(0x13, R.string.vendor_name_dianjing),
			new VendorNameMap(0x30, R.string.vendor_name_simcom),
		}),
		new VendorMap("hs8801", new VendorNameMap[] {
			new VendorNameMap(0x58, R.string.vendor_name_lihe),
			new VendorNameMap(0x59, R.string.vendor_name_dianjing),
			new VendorNameMap(0x32, R.string.vendor_name_lianchuang),
		}),
	};

	private int mVendorID;
	private int mFwVersion;

	private VendorMap mVendorMap;
	private VendorNameMap mNameMap;
	private VendorShortNameMap mShortNameMap;

	public static final Creator<DeviceID> CREATOR = new Creator<DeviceID>() {
		@Override
		public DeviceID[] newArray(int size) {
			return new DeviceID[size];
		}

		@Override
		public DeviceID createFromParcel(Parcel source) {
			return new DeviceID(source.readInt(), source.readInt());
		}
	};

	public DeviceID(int vendorID, int fwVersion) {
		super();
		mVendorID = vendorID;
		mFwVersion = fwVersion;

		Log.d(TAG, "mVendorID = " + mVendorID);
		Log.d(TAG, "mFwVersion = " + mFwVersion);

		mVendorMap = VendorMap.findMapByProjectName(mVendorMaps, Build.BOARD);
		if (mVendorMap != null) {
			mNameMap = VendorNameMap.findMapByVendorId(mVendorMap.getNameMaps(), mVendorID);
			if (mNameMap != null) {
				mShortNameMap = VendorShortNameMap.findMapByResource(mShortNameMaps, mNameMap.getResourceId());
			}
		}
	}

	public int getVendorID() {
		return mVendorID;
	}

	public int getVendorNameResId() {
		if (mNameMap == null) {
			return R.string.vendor_name_unknown;
		}

		return mNameMap.getResourceId();
	}

	public String getVendorShortName() {
		if (mShortNameMap == null) {
			return "unknown";
		}

		return mShortNameMap.getShortName();
	}

	public int getFwVersion() {
		return mFwVersion;
	}

	@Override
	public int describeContents() {
		return 0;
	}

	@Override
	public void writeToParcel(Parcel dest, int flags) {
		dest.writeInt(mVendorID);
		dest.writeInt(mFwVersion);
	}
}
