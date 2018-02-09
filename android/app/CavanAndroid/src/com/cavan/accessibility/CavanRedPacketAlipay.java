package com.cavan.accessibility;

public class CavanRedPacketAlipay extends CavanRedPacket {

	private String mCode;

	private int mPostCount;
	private int mCommitCount;
	private boolean mPostPending;

	private long mExactTime;
	private long mRepeatTime;

	private int mPriority;
	private boolean mValid;
	private boolean mInvalid;
	private boolean mIgnored;
	private boolean mCompleted;
	private boolean mRepeatable;
	private boolean mMaybeInvalid;

	public CavanRedPacketAlipay(String code) {
		mCode = code;
	}

	public String getCode() {
		return mCode;
	}

	public void setCode(String code) {
		mCode = code;
	}

	public synchronized int getPostCount() {
		return mPostCount;
	}

	public synchronized void setPostCount(int count) {
		mPostCount = count;
	}

	public synchronized int getCommitCount() {
		return mCommitCount;
	}

	public synchronized void setCommitCount(int count) {
		mCommitCount = count;
	}

	public synchronized boolean isPostPending() {
		return mPostPending;
	}

	public synchronized void setPostPending(boolean pending) {
		mPostPending = pending;
	}

	public synchronized long getExactTime() {
		return mExactTime;
	}

	public synchronized void setExactTime(long time) {
		mExactTime = time;
	}

	public synchronized long getRepeatTime() {
		return mRepeatTime;
	}

	public synchronized void setRepeatTime(long time) {
		mRepeatTime = time;
	}

	public synchronized boolean updateRepeatTime() {
		long timeNow = System.currentTimeMillis();

		if (mRepeatTime < timeNow) {
			if (mRepeatTime != 0) {
				if (++mCommitCount > 5) {
					return ((timeNow - mRepeatTime) < 20000);
				}
			} else {
				mRepeatTime = ((timeNow - 5000) / 60000 + 1) * 60000;
				mUnpackTime = mRepeatTime;
				mCommitCount = 0;
			}
		}

		return true;
	}

	public synchronized int getPriority() {
		return mPriority;
	}

	public synchronized void setPriority(int priority) {
		mPriority = priority;
	}

	public synchronized boolean isValid() {
		return mValid;
	}

	public synchronized void setValid(boolean valid) {
		mValid = valid;
	}

	public synchronized boolean isInvalid() {
		return mInvalid;
	}

	public synchronized void setInvalid(boolean invalid) {
		mInvalid = invalid;
	}

	public synchronized boolean isIgnored() {
		return mIgnored;
	}

	public synchronized void setIgnored(boolean ignored) {
		mIgnored = ignored;
	}

	public synchronized boolean isCompleted() {
		return mCompleted;
	}

	public synchronized void setCompleted(boolean completed) {
		mCompleted = completed;
	}

	public synchronized boolean isRepeatable() {
		return mRepeatable;
	}

	public synchronized void setRepeatable() {
		mRepeatable = true;
		mValid = true;
	}

	public synchronized boolean isMaybeInvalid() {
		if (mValid) {
			return false;
		}

		return mMaybeInvalid;
	}

	public synchronized void setMaybeInvalid(boolean invalid) {
		mMaybeInvalid = invalid;
	}

	@Override
	public boolean equals(Object o) {
		if (super.equals(o)) {
			return true;
		}

		if (o instanceof CavanRedPacketAlipay) {
			return mCode.equals(((CavanRedPacketAlipay) o).getCode());
		}

		if (o instanceof String) {
			return mCode.equals(o);
		}

		return false;
	}

}
