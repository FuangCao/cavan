package com.cavan.cavanmain;

import java.text.SimpleDateFormat;
import java.util.Date;

import android.annotation.SuppressLint;
import android.os.Parcel;
import android.os.Parcelable;

@SuppressLint("SimpleDateFormat")
public class RedPacketCode implements Parcelable {

	private static final SimpleDateFormat sDateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

	private long mTime;
	private String mCode;

	public RedPacketCode(String code, long delay) {
		super();

		mTime = System.currentTimeMillis() + delay;
		mCode = code;
	}

	public RedPacketCode(String code) {
		mCode = code;
	}

	public long getTime() {
		return mTime;
	}

	public void setTime(long time) {
		mTime = time;
	}

	public String getCode() {
		return mCode;
	}

	public void setCode(String code) {
		mCode = code;
	}

	public long getDelay() {
		long time = System.currentTimeMillis();
		if (mTime > time) {
			return mTime - time;
		}

		return 0;
	}

	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder(mCode);

		if (mTime > 0) {
			builder.append(", time = ");
			builder.append(sDateFormat.format(new Date(mTime)));
		}

		return builder.toString();
	}

	@Override
	public int describeContents() {
		return 0;
	}

	@Override
	public void writeToParcel(Parcel dest, int flags) {
		dest.writeString(mCode);
		dest.writeLong(mTime);
	}

	public static final Creator<RedPacketCode> CREATOR = new Creator<RedPacketCode>() {

		@Override
		public RedPacketCode[] newArray(int size) {
			return new RedPacketCode[size];
		}

		@Override
		public RedPacketCode createFromParcel(Parcel source) {
			RedPacketCode code = new RedPacketCode(source.readString());
			code.setTime(source.readLong());
			return code;
		}
	};
}
