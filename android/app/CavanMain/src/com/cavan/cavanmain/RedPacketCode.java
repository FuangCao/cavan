package com.cavan.cavanmain;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;

@SuppressLint("SimpleDateFormat")
public class RedPacketCode {

	private static long CODE_OVERTIME = 28800000;
	private static long REPEAT_TIME_ALIGN = 60000;

	private static final SimpleDateFormat sDateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
	private static HashMap<String, RedPacketCode> mCodeMap = new HashMap<String, RedPacketCode>();

	private int mPostCount;
	private int mCommitCount;
	private boolean mPostPending;

	private long mTime;
	private long mRepeatTime;

	private String mCode;
	private boolean mValid;
	private boolean mInvalid;
	private boolean mShared;
	private boolean mCompleted;
	private boolean mMaybeInvalid;

	public static RedPacketCode getInstence(String code, boolean create) {
		Iterator<RedPacketCode> iterator = mCodeMap.values().iterator();
		while (iterator.hasNext()) {
			RedPacketCode node = iterator.next();
			if (node.getTimeout() > CODE_OVERTIME) {
				iterator.remove();
			}
		}

		RedPacketCode node = mCodeMap.get(code);
		if (node == null && create) {
			node = new RedPacketCode(code);
			mCodeMap.put(code, node);
		}

		return node;
	}

	public static RedPacketCode getInstence(Intent intent) {
		String code = intent.getStringExtra("code");
		if (code == null) {
			return null;
		}

		return getInstence(code, false);
	}

	private RedPacketCode(String code) {
		mCode = code;
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
		mValid = true;
	}

	public boolean isValid() {
		return mValid;
	}

	public void setInvalid() {
		mInvalid = true;
	}

	public boolean isInvalid() {
		if (mValid) {
			return false;
		}

		return mInvalid;
	}

	public void setShared() {
		mShared = true;
	}

	public boolean isShared() {
		return mShared;
	}

	public boolean isRepeatable() {
		return mRepeatTime > 0;
	}

	public void setRepeatable(Context context) {
		mRepeatTime = System.currentTimeMillis() + REPEAT_TIME_ALIGN; // - 1;
		mRepeatTime -= mRepeatTime % REPEAT_TIME_ALIGN;
		mTime = mRepeatTime - MainActivity.getCommitAhead(context);
		mValid = true;
	}

	public long getRepeatTimeout() {
		return System.currentTimeMillis() - mRepeatTime;
	}

	public void setCompleted() {
		mCompleted = true;
	}

	public boolean isCompleted() {
		if (mRepeatTime > 0) {
			return false;
		}

		return mCompleted;
	}

	public int getCommitCount() {
		if (mValid) {
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
		if (mValid) {
			return false;
		}

		return mMaybeInvalid && mPostPending;
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
			builder.append(", ");
			builder.append(sDateFormat.format(new Date(mTime)));
		}

		if (mShared) {
			builder.append(", Shared");
		}

		if (mCompleted) {
			builder.append(", Completed");
		} else if (mRepeatTime > 0) {
			builder.append(", Repeatable");
		} else if (mValid) {
			builder.append(", Valid");
		}

		return builder.toString();
	}
}
