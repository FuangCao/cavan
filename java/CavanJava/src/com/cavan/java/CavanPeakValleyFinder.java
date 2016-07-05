package com.cavan.java;

public class CavanPeakValleyFinder extends CavanPeakValleyValue {

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

		if (mFindPeak) {
			if (value > mPeakValue) {
				mPeakValue = value;
			} else {
				mFindPeak = false;
				mPeakFound = true;

				if (mValleyFound) {
					mValleyFound = false;
					result = new CavanPeakValleyValue(mPeakValue, mValleyValue);
				}

				mFindValley = true;
				mValleyValue = value;
			}
		} else if (mFindValley) {
			if (value < mValleyValue) {
				mValleyValue = value;
			} else {
				mValleyFound = true;

				if (mPeakFound) {
					mPeakFound = false;
					result = new CavanPeakValleyValue(value, mValleyValue);
				}

				mFindPeak = true;
				mPeakValue = value;
			}
		} else if (mInitialized) {
			if (value > mPeakValue) {
				mFindPeak = true;
				mPeakValue = value;
			} else if (value < mValleyValue) {
				mFindValley = true;
				mValleyValue = value;
			} else {
				mPeakValue = mValleyValue = value;
			}
		} else {
			mInitialized = true;
			mPeakValue = mValleyValue = value;
		}

		return result;
	}
}