package com.cavan.touchscreen;

import android.os.Build;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

public class DeviceID implements Parcelable {
	private static final String TAG = "Cavan";

	private int mVendorID;
	private int mVendorName;
	private int mFwVersion;

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

		if (Build.BOARD.equals("h1")) {
			switch (mVendorID) {
			case 0x13:
				mVendorName = R.string.vendor_name_dianjing;
				break;

			case 0x31:
				mVendorName = R.string.vendor_name_yeji;
				break;

			default:
				mVendorName = R.string.vendor_name_unknown;
			}
		} else if (Build.BOARD.equals("h2")) {
			switch (mVendorID) {
			case 0x94:
				mVendorName = R.string.vendor_name_yingmao;
				break;

			case 0x87:
				mVendorName = R.string.vendor_name_lianchuang;
				break;

			case 0x11:
				mVendorName = R.string.vendor_name_lihaojie;
				break;

			default:
				mVendorName = R.string.vendor_name_unknown;
			}
		} else if (Build.BOARD.equals("h3")) {
			switch (mVendorID) {
			case 0xdb:
				mVendorName = R.string.vendor_name_dianjing;
				break;

			case 0xda:
				mVendorName = R.string.vendor_name_tongxincheng;
				break;

			default:
				mVendorName = R.string.vendor_name_unknown;
			}
		} else if (Build.BOARD.equals("h4")) {
			switch (mVendorID) {
			case 0xba:
				mVendorName = R.string.vendor_name_dianjing;
				break;

			case 0x11:
				mVendorName = R.string.vendor_name_lihaojie;
				break;

			case 0x74:
				mVendorName = R.string.vendor_name_yuansheng;
				break;

			default:
				mVendorName = R.string.vendor_name_unknown;
			}
		} else if (Build.BOARD.equals("h5")) {
			switch (mVendorID) {
			case 0x32:
				mVendorName = R.string.vendor_name_lianchuang;
				break;

			case 0x13:
				mVendorName = R.string.vendor_name_dianjing;
				break;

			default:
				mVendorName = R.string.vendor_name_unknown;
			}
		} else {
			mVendorName = R.string.vendor_name_unknown;
		}
	}

	public int getVendorID() {
		return mVendorID;
	}

	public int getVendorName() {
		return mVendorName;
	}

	public String getVendorShortName() {
		switch (mVendorName) {
		case R.string.vendor_name_dianjing:
			return "dianjing";
		case R.string.vendor_name_lianchuang:
			return "lianchuang";
		case R.string.vendor_name_lihaojie:
			return "lihaojie";
		case R.string.vendor_name_simcom:
			return "simcom";
		case R.string.vendor_name_tongxincheng:
			return "tongxincheng";
		case R.string.vendor_name_yeji:
			return "yeji";
		case R.string.vendor_name_yingmao:
			return "yingmao";
		case R.string.vendor_name_yuansheng:
			return "yuansheng";
		default:
			return null;
		}
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
