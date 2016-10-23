package com.cavan.cavanmain;

import java.text.SimpleDateFormat;
import java.util.Date;

import android.annotation.SuppressLint;
import android.os.Parcel;
import android.os.Parcelable;

import com.cavan.java.CavanJava;

@SuppressLint("SimpleDateFormat")
public class RedPacketCode implements Parcelable {

	private static final SimpleDateFormat sDateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

	private int mPostCount;
	private int mCommitCount;
	private boolean mPostPending;

	private long mTime;
	private long mRepeatTime;

	private String mCode;
	private boolean mIsValid;
	private boolean mNetShared;
	private boolean mCompleted;
	private boolean mMaybeInvalid;

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

	public long getTimeout() {
		long time = System.currentTimeMillis();
		if (mTime < time) {
			return time - mTime;
		}

		return 0;
	}

	public void setValid() {
		mIsValid = true;
	}

	public boolean isValid() {
		return mIsValid;
	}

	public void setNetShared() {
		mNetShared = true;
	}

	public boolean isNetShared() {
		return mNetShared;
	}

	public void setRepeatable(long align, long ahead) {
		mRepeatTime = System.currentTimeMillis() + align - 1;
		mRepeatTime -= mRepeatTime % align;
		mTime = mRepeatTime - ahead;
		mIsValid = true;
	}

	public boolean isRepeatable() {
		return mRepeatTime > 0;
	}

	public long getRepeatTimeout() {
		return System.currentTimeMillis() - mRepeatTime;
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
		mPostPending = false;
		mCommitCount = count;
	}

	public boolean canRemove() {
		return mPostCount > 0;
	}

	public void setPostPending(boolean pending) {
		mPostPending = pending;
		mMaybeInvalid = false;
	}

	public boolean maybeInvalid() {
		if (mIsValid) {
			return false;
		}

		return mMaybeInvalid;
	}

	public boolean setPostComplete() {
		if (!mPostPending) {
			return false;
		}

		setCommitCount(mCommitCount + 1);
		mMaybeInvalid = true;
		mPostCount++;

		return true;
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

		if (mRepeatTime > 0) {
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
