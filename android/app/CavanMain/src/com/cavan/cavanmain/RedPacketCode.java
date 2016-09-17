package com.cavan.cavanmain;

import java.text.SimpleDateFormat;
import java.util.Date;

import com.cavan.java.CavanJava;

import android.annotation.SuppressLint;
import android.os.Parcel;
import android.os.Parcelable;

@SuppressLint("SimpleDateFormat")
public class RedPacketCode implements Parcelable {

	private static final SimpleDateFormat sDateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

	private int mCommitCount;

	private long mTime;
	private String mCode;
	private boolean mNetShared;
	private boolean mRepeatable;

	public RedPacketCode(String code) {
		mCode = code;
	}

	public RedPacketCode(String code, long time, boolean shared) {
		mCode = code;
		mTime = time;
		mNetShared = shared;
	}

	public long getTime() {
		return mTime;
	}

	public void setTime(long time) {
		mTime = time;
	}

	public long updateTime() {
		long time = System.currentTimeMillis();

		if (mTime > time) {
			return mTime - time;
		}

		mTime = time;

		return 0;
	}

	public String getCode() {
		return mCode;
	}

	public void setCode(String code) {
		mCode = code;
	}

	public void setDelay(long delay) {
		mTime = System.currentTimeMillis() + delay;
	}

	public long getDelay() {
		long time = System.currentTimeMillis();
		if (mTime > time) {
			return mTime - time;
		}

		return 0;
	}

	public void setRepeatable(boolean enable) {
		mRepeatable = enable;
		updateTime();
	}

	public void setRepeatable() {
		setRepeatable(true);
	}

	public boolean isRepeatable() {
		return mRepeatable;
	}

	public int getCommitCount() {
		if (mRepeatable) {
			return 0;
		}

		return mCommitCount;
	}

	public int addCommitCount() {
		if (mRepeatable) {
			return 1;
		}

		return ++mCommitCount;
	}

	public int subCommitCount() {
		if (mCommitCount > 0) {
			return --mCommitCount;
		}

		return 0;
	}

	public void setNetShared() {
		mNetShared = true;
	}

	public boolean isNetShared() {
		return mNetShared;
	}

	@Override
	public boolean equals(Object obj) {
		if (mCode == obj) {
			return true;
		}

		if (obj instanceof RedPacketCode) {
			obj = ((RedPacketCode) obj).getCode();
		} else if (obj != null) {
			obj = obj.toString();
		} else {
			return false;
		}

		return mCode.equals(obj);
	}

	@Override
	public int hashCode() {
		return mCode.hashCode();
	}

	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder();

		builder.append("code = ");
		builder.append(mCode);

		if (mTime > 0) {
			builder.append(", time = ");
			builder.append(sDateFormat.format(new Date(mTime)));
		}

		builder.append(", repeatable = ");
		builder.append(mRepeatable);

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
		dest.writeByte(CavanJava.getBoolValueByte(mNetShared));
	}

	public static final Creator<RedPacketCode> CREATOR = new Creator<RedPacketCode>() {

		@Override
		public RedPacketCode[] newArray(int size) {
			return new RedPacketCode[size];
		}

		@Override
		public RedPacketCode createFromParcel(Parcel source) {
			return new RedPacketCode(source.readString(), source.readLong(), source.readByte() > 0);
		}
	};
}
