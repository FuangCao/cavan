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
	private long mCommitTime;

	private long mTime;
	private String mCode;
	private boolean mIsValid;
	private boolean mNetShared;
	private boolean mRepeatable;
	private boolean mCompleted;

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

		mCommitTime = 0;

		if (mTime > time) {
			return mTime - time;
		}

		mTime = time;

		return 0;
	}

	public void alignTime(long align, long ahead) {
		mTime = ((System.currentTimeMillis() + (align - 1)) / align) * align - ahead;
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

	public long getTimeout() {
		long time = System.currentTimeMillis();
		if (mTime < time) {
			return time - mTime;
		}

		return 0;
	}

	public void setRepeatable() {
		mRepeatable = true;
		mIsValid = true;
		updateTime();
	}

	public boolean isRepeatable() {
		return mRepeatable;
	}

	public void setCompleted() {
		mCompleted = true;
	}

	public boolean isCompleted() {
		return mCompleted;
	}

	public int getCommitCount() {
		if (mIsValid) {
			return 0;
		}

		return mCommitCount;
	}

	public void setCommitCount(int count) {
		mCommitCount = count;
	}

	public int addCommitCount() {
		mCommitTime = System.currentTimeMillis();

		if (mIsValid) {
			mCommitCount = 1;
		} else {
			mCommitCount++;
		}

		return mCommitCount;
	}

	public int subCommitCount() {
		if (mCommitCount > 0) {
			return --mCommitCount;
		}

		return 0;
	}

	public boolean canComplete() {
		return mCommitCount > 0;
	}

	public void setNetShared() {
		mNetShared = true;
	}

	public boolean isNetShared() {
		return mNetShared;
	}

	public void setValid() {
		mIsValid = true;
	}

	public boolean isValid() {
		return mIsValid;
	}

	public boolean isCommited() {
		return mCommitTime != 0;
	}

	public long getCommitTimeConsume() {
		return System.currentTimeMillis() - mCommitTime;
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
		StringBuilder builder = new StringBuilder(mCode);

		if (mTime > 0) {
			builder.append(" ");
			builder.append(sDateFormat.format(new Date(mTime)));
		}

		if (mNetShared) {
			builder.append(" Shared");
		}

		if (mRepeatable) {
			builder.append(" Repeatable");
		}

		if (mIsValid) {
			builder.append(" Valid");
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
