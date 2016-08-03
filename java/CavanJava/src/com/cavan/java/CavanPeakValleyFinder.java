package com.cavan.java;


public class CavanPeakValleyFinder extends CavanPeakValleyValue {

	protected boolean mFindPeak;
	protected boolean mFindValley;

	protected boolean mPeakFound;
	protected boolean mValleyFound;

	protected boolean mInitialized;
	protected double mLastValue;
	private double mAvgValue;

	public CavanPeakValleyFinder() {
		super(0);
	}

	public boolean putValue(double value) {
		boolean found = false;

		if (mFindPeak) {
			if (value < mLastValue) {
				mTime = System.currentTimeMillis();
				mPeakValue = mLastValue;
				mType = TYPE_RISING;

				mPeakFound = true;
				found = mValleyFound;

				mFindPeak = false;
			}
		} else if (mFindValley) {
			if (value > mLastValue) {
				mTime = System.currentTimeMillis();
				mValleyValue = mLastValue;
				mType = TYPE_FALLING;

				mValleyFound = true;
				found = mPeakFound;

				mFindPeak = true;
			}
		} else if (mInitialized) {
			if (value > mLastValue) {
				mFindPeak = true;
			}

			mFindValley = true;
		} else {
			mInitialized = true;
		}

		mLastValue = value;

		return found;
	}

	public boolean putAvgValue(double value) {
		mAvgValue = (mAvgValue + value) / 2;
		return putValue(mAvgValue);
	}
}