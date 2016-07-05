package com.cavan.java;

public class CavanPeakValleyFinder extends CavanPeakValleyValue {

	private double mValue;
	private boolean mFindPeak;
	private boolean mFindValley;
	private boolean mPeakFound;
	private boolean mValleyFound;
	private boolean mInitialized;

	public CavanPeakValleyFinder() {
		super(0, 0);
	}

	public CavanPeakValleyFinder(double value) {
		super(value, value);
		mInitialized = true;
	}

	public CavanPeakValleyValue putValue(double value) {
		CavanPeakValleyValue result = null;

		mValue = (mValue + value) / 2;

		if (mFindPeak) {
			if (mValue > mPeakValue) {
				mPeakValue = mValue;
			} else {
				mFindPeak = false;
				mPeakFound = true;

				if (mValleyFound) {
					mValleyFound = false;
					result = new CavanPeakValleyValue(mPeakValue, mValleyValue);
				}

				mFindValley = true;
				mValleyValue = mValue;
			}
		} else if (mFindValley) {
			if (mValue < mValleyValue) {
				mValleyValue = mValue;
			} else {
				mValleyFound = true;

				if (mPeakFound) {
					mPeakFound = false;
					result = new CavanPeakValleyValue(mPeakValue, mValleyValue);
				}

				mFindPeak = true;
				mPeakValue = mValue;
			}
		} else if (mInitialized) {
			if (mValue > mPeakValue) {
				mFindPeak = true;
				mPeakValue = mValue;
			} else if (mValue < mValleyValue) {
				mFindValley = true;
				mValleyValue = mValue;
			} else {
				mPeakValue = mValleyValue = mValue;
			}
		} else {
			mInitialized = true;
			mPeakValue = mValleyValue = mValue;
		}

		return result;
	}
}