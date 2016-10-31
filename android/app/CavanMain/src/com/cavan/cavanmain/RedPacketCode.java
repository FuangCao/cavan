package com.cavan.cavanmain;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;

@SuppressLint("SimpleDateFormat")
public class RedPacketCode {

	private static int LAST_CODE_SIZE = 8;
	private static long LAST_CODE_OVERTIME = 300000;
	private static long CODE_OVERTIME = 28800000;
	private static long REPEAT_TIME_ALIGN = 60000;

	private static final SimpleDateFormat sDateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

	private static HashMap<String, RedPacketCode> mCodeMap = new HashMap<String, RedPacketCode>();
	private static LinkedList<RedPacketCode> mLastCodes = new LinkedList<RedPacketCode>();

	private int mPostCount;
	private int mCommitCount;
	private boolean mPostPending;

	private long mTime;
	private long mRepeatTime;

	private String mCode;
	private boolean mValid;
	private boolean mInvalid;
	private boolean mShared;
	private boolean mTestOnly;
	private boolean mCompleted;
	private boolean mRepeatable;
	private boolean mMaybeInvalid;

	public static RedPacketCode getInstence(String code, boolean create, boolean test) {
		synchronized (mCodeMap) {
			Iterator<RedPacketCode> iterator = mCodeMap.values().iterator();
			while (iterator.hasNext()) {
				RedPacketCode node = iterator.next();
				if (node.getTimeout() > CODE_OVERTIME) {
					iterator.remove();
				}
			}

			RedPacketCode node = mCodeMap.get(code);
			if (node == null) {
				if (create) {
					node = new RedPacketCode(code);
					mCodeMap.put(code, node);
				} else {
					return null;
				}
			}

			if (test) {
				node.setTestOnly();
			} else if (create) {
				synchronized (mLastCodes) {
					mLastCodes.remove(node);
					mLastCodes.addFirst(node);
					updateLastCodes();
				}
			}

			return node;
		}
	}

	public static RedPacketCode getInstence(Intent intent) {
		String code = intent.getStringExtra("code");
		if (code == null) {
			return null;
		}

		return getInstence(code, false, false);
	}

	public static List<RedPacketCode> getLastCodes() {
		synchronized (mLastCodes) {
			updateLastCodes();
			return mLastCodes;
		}
	}

	public static int updateLastCodes() {
		int count = 0;

		synchronized (mLastCodes) {
			while (mLastCodes.size() > LAST_CODE_SIZE && mLastCodes.getLast().getTimeout() > LAST_CODE_OVERTIME) {
				mLastCodes.removeLast();
				count++;
			}
		}

		return count;
	}

	private RedPacketCode(String code) {
		mCode = code;
		mTime = System.currentTimeMillis();
	}

	synchronized public long getTime() {
		return mTime;
	}

	synchronized public void setTime(long time) {
		mTime = time;
	}

	synchronized public long updateTime() {
		long time = System.currentTimeMillis();

		if (mTime > time) {
			return mTime - time;
		}

		mTime = time;

		return 0;
	}

	synchronized public String getCode() {
		return mCode;
	}

	synchronized public void setCode(String code) {
		mCode = code;
	}

	synchronized public void setTestOnly() {
		mTestOnly = true;
	}

	synchronized public boolean isTestOnly() {
		return mTestOnly;
	}

	synchronized public void setDelay(long delay) {
		mTime = System.currentTimeMillis() + delay;
	}

	synchronized public long getDelay() {
		long time = System.currentTimeMillis();
		if (mTime > time) {
			return mTime - time;
		}

		return 0;
	}

	synchronized public long getTimeout() {
		long time = System.currentTimeMillis();
		if (mTime < time) {
			return time - mTime;
		}

		return 0;
	}

	synchronized public void setValid() {
		mValid = true;
		mInvalid = false;
		mMaybeInvalid = false;
	}

	synchronized public boolean isValid() {
		return mValid;
	}

	synchronized public void setInvalid() {
		mInvalid = true;
	}

	synchronized public boolean isInvalid() {
		if (mValid) {
			return false;
		}

		return mInvalid;
	}

	synchronized public void setShared() {
		mShared = true;
	}

	synchronized public boolean isShared() {
		return mShared;
	}

	synchronized public boolean isRepeatable() {
		return mRepeatable;
	}

	synchronized public void setRepeatable() {
		mRepeatable = true;
	}

	synchronized public void updateRepeatTime(Context context) {
		mValid = true;
		mRepeatable = true;
		mRepeatTime = System.currentTimeMillis() + REPEAT_TIME_ALIGN;
		mRepeatTime -= mRepeatTime % REPEAT_TIME_ALIGN;
		mTime = mRepeatTime - MainActivity.getCommitAhead(context);
	}

	synchronized public long getRepeatTime() {
		if (mRepeatable) {
			return mRepeatTime;
		}

		return 0;
	}

	synchronized public long getRepeatTimeout() {
		return System.currentTimeMillis() - mRepeatTime;
	}

	synchronized public void setCompleted() {
		mCompleted = true;
	}

	synchronized public boolean isCompleted() {
		if (mRepeatTime > 0) {
			return false;
		}

		return mCompleted;
	}

	synchronized public int getCommitCount() {
		if (mValid) {
			return 0;
		}

		return mCommitCount;
	}

	synchronized public void setCommitCount(int count) {
		mPostPending = false;
		mInvalid = false;
		mMaybeInvalid = false;
		mCommitCount = count;
	}

	synchronized public boolean canRemove() {
		return mPostCount > 0;
	}

	synchronized public void setPostPending(boolean pending) {
		mPostPending = pending;
		mMaybeInvalid = false;
	}

	synchronized public boolean maybeInvalid() {
		if (mValid) {
			return false;
		}

		return mMaybeInvalid;
	}

	synchronized public boolean setPostComplete() {
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
		} else if (mInvalid) {
			builder.append(", Invalid");
		} else if (mMaybeInvalid) {
			builder.append(", MaybeInvalid");
		}

		return builder.toString();
	}
}
