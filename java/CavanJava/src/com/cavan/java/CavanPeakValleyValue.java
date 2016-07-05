package com.cavan.java;

public class CavanPeakValleyValue {
	protected double mPeakValue;
	protected double mValleyValue;

	public CavanPeakValleyValue(double peak, double valley) {
		super();

		mPeakValue = peak;
		mValleyValue = valley;
	}

	public double getPeakValue() {
		return mPeakValue;
	}

	public void setPeakValue(double peak) {
		mPeakValue = peak;
	}

	public double getValleyValue() {
		return mValleyValue;
	}

	public void setValleyValue(double valley) {
		mValleyValue = valley;
	}

	public double getDiff() {
		return mPeakValue - mValleyValue;
	}

	public void extend(CavanPeakValleyValue value) {
		if (value.getPeakValue() > mPeakValue) {
			mPeakValue = value.getPeakValue();
		}

		if (value.getValleyValue() < mValleyValue) {
			mValleyValue = value.getValleyValue();
		}
	}
}
