package com.cavan.cavanmain;

public class ThanksNode {

	private long mTime;
	private int mCount;

	public long getTime() {
		return mTime;
	}

	public void setTime(long time) {
		mTime = time;
	}

	public long getCount() {
		return mCount;
	}

	public void setCount(int count) {
		mCount = count;
	}

	public int increase(long time)
	{
		mTime = time;
		return ++mCount;
	}
}
