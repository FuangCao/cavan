package com.cavan.java;

import java.util.ArrayList;
import java.util.List;

public class CavanPeakValleyFinder extends CavanPeakValleyValue {

	public static final long FREQ_TIMEOUT = 2000;
	public static final long FREQ_COUNT = 10;

	private double mAvgDiff;
	private double mAvgValue;

	private boolean mFindPeak;
	private boolean mFindValley;
	private boolean mInitialized;

	private long mTimeFuzz;
	private long mPeakTime;
	private long mValleyTime;
	private long mLastPeakTime;
	private long mLastValleyTime;

	private double mValueFuzz;
	private double mValueFuzzMin;
	private double mLastValue;
	private double mLastPeak;
	private double mLastValley;

	private int mFreq;
	private List<CavanPeakValleyValue> mValueList = new ArrayList<CavanPeakValleyValue>();

	public CavanPeakValleyFinder(long timeFuzz, double valueFuzz) {
		super(0);

		setTimeFuzz(timeFuzz);
		setValueFuzz(valueFuzz);
	}

	public CavanPeakValleyFinder(double value, long timeFuzz, double valueFuzz) {
		super(value);

		setTimeFuzz(timeFuzz);
		setValueFuzz(valueFuzz);
		mInitialized = true;
	}

	protected void onFreqChanged(int freq) {
		// CavanAndroid.logE("freq = " + freq);
	}

	public void setValueFuzz(double fuzz) {
		mValueFuzz = fuzz;
		mValueFuzzMin = fuzz / 3;
	}

	public void setTimeFuzz(long fuzz) {
		mTimeFuzz = fuzz;
	}

	public void init(double value) {
		mPeakValue = mValleyValue = mLastPeak = mLastValley = value;
		mPeakTime = mValleyTime = mLastPeakTime = mLastValleyTime = System.currentTimeMillis();
	}

	public double getAvgDiff() {
		return mAvgDiff;
	}

	public double getAvgValue() {
		return mAvgValue;
	}

	public CavanPeakValleyValue createPeakValley(int type, long time) {
		if (mType == type) {
			return null;
		}

		mType = type;

		CavanPeakValleyValue result = new CavanPeakValleyValue(mLastPeak, mLastValley, type, time);
		mAvgDiff = (mAvgDiff * 2 + result.getDiff()) / 3;

		return result;
	}

	public boolean isValidPeakValley(long time) {
		double diff = getDiff();
		if (diff > mValueFuzz) {
			return true;
		}

		return diff > mValueFuzzMin && time > mTimeFuzz;
	}

	public CavanPeakValleyValue putAvgValue(double value) {
		mAvgValue = value;
		CavanPeakValleyValue result = null;

		if (mFindPeak) {
			if (value < mLastValue) {
				mPeakValue = mLastValue;
				mPeakTime = System.currentTimeMillis();

				if (isValidPeakValley(mPeakTime - mValleyTime)) {
					result = createPeakValley(TYPE_FALLING, mLastValleyTime);
					mLastPeak = mPeakValue;
					mLastPeakTime = mPeakTime;
				}

				mFindPeak = false;
				mFindValley = true;
			}
		} else if (mFindValley) {
			if (value > mLastValue) {
				mValleyValue = mLastValue;
				mValleyTime = System.currentTimeMillis();

				if (isValidPeakValley(mValleyTime - mPeakTime)) {
					result = createPeakValley(TYPE_RISING, mLastPeakTime);
					mLastValley = mValleyValue;
					mLastValleyTime = mValleyTime;
				}

				mFindPeak = true;
			}
		} else if (mInitialized) {
			if (value > mLastValue) {
				mFindPeak = true;
			} else if (value < mLastValue) {
				mFindValley = true;
			} else {
				init(value);
			}
		} else {
			init(value);
			mInitialized = true;
		}

		mLastValue = value;

		return result;
	}

	public CavanPeakValleyValue putValue(double value) {
		return putAvgValue((mAvgValue + value) / 2);
	}

	public int getFreq() {
		return mFreq;
	}

	private void setFreq(int freq) {
		if (mFreq != freq) {
			mFreq = freq;
			onFreqChanged(freq);
		}
	}

	private void setFreq(CavanPeakValleyValue first, long time, int count) {
		setFreq((int) ((count * 30000) / first.getTimeEarly(time)));
	}

	private void updateFreq(long time) {
		int count = mValueList.size();
		if (count > 1) {
			setFreq(mValueList.get(0), time, count - 1);
		} else {
			setFreq(0);
		}
	}

	public CavanPeakValleyValue putFreqValue(double value) {
		CavanPeakValleyValue result = putValue(value);
		if (result != null) {
			mTime = result.getTime();

			while (mValueList.size() > FREQ_COUNT) {
				mValueList.remove(0);
			}

			mValueList.add(result);
			updateFreq(mTime);
		} else {
			long time = System.currentTimeMillis();
			if (time - mTime > FREQ_TIMEOUT) {
				mValueList.clear();
				setFreq(0);
			} else {
				updateFreq(time);
			}
		}

		return result;
	}
}
