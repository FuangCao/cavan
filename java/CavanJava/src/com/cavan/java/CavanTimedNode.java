package com.cavan.java;

public class CavanTimedNode<E> {

	private E mValue;
	private long mTime;

	public CavanTimedNode(E value, long time) {
		mTime = time;
		mValue = value;
	}

	public CavanTimedNode(E value) {
		this(value, System.currentTimeMillis());
	}

	public long getTime() {
		return mTime;
	}

	public void setTime(long time) {
		mTime = time;
	}

	public E getValue() {
		return mValue;
	}

	public void setValue(E value) {
		mValue = value;
	}
}
