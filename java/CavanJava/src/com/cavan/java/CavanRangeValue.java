package com.cavan.java;

public class CavanRangeValue {

	private double mMinValue;
	private double mMaxValue;

	public CavanRangeValue(double min, double max) {
		setRange(min, max);
	}

	public double getMinValue() {
		return mMinValue;
	}

	public void setMinValue(double min) {
		mMinValue = min;
	}

	public double getMaxValue() {
		return mMaxValue;
	}

	public void setMaxValue(double max) {
		mMaxValue = max;
	}

	public void setRange(double min, double max) {
		mMinValue = min;
		mMaxValue = max;
	}

	public double getRange() {
		return mMaxValue - mMinValue;
	}

	public double getAvgValue() {
		return (mMaxValue + mMinValue) / 2;
	}

	@Override
	public String toString() {
		return String.format("[%4.2f, %4.2f]", mMinValue, mMaxValue);
	}
}