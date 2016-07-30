package com.cavan.java;

public class CavanWaveValue {

	private double mValue;
	private long mTime;

	public CavanWaveValue(double value, long time) {
		mValue = value;
		mTime = time;
	}

	public CavanWaveValue(double value) {
		this(value, System.currentTimeMillis());
	}

	public double getValue() {
		return mValue;
	}

	public void setValue(double value) {
		mValue = value;
	}

	public long getTime() {
		return mTime;
	}

	public void setTime(long time) {
		mTime = time;
	}
}
