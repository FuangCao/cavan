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
		mValueFuzzMin = fuzz / 4;
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

	public CavanPeakValleyValue buildValue(double peak, double valley, int type) {
		CavanPeakValleyValue result = new CavanPeakValleyValue(peak, valley, type);
		mAvgDiff = (mAvgDiff * 2 + result.getDiff()) / 3;
		return result;
	}

	public CavanPeakValleyValue buildValueRising() {
		return buildValue(mPeakValue, mLastValley, TYPE_RISING);
	}

	public CavanPeakValleyValue buildValueFalling() {
		return buildValue(mLastPeak, mValleyValue, TYPE_FALLING);
	}

	public CavanPeakValleyValue putAvgValue(double value) {
		mAvgValue = value;
		CavanPeakValleyValue result = null;

		if (mFindPeak) {
			if (value > mPeakValue) {
				mPeakValue = value;
			} else {
				mPeakTime = System.currentTimeMillis();

				double diff = getDiff();

				if (diff > mValueFuzz || (diff > mValueFuzzMin &&  mPeakTime - mValleyTime > mTimeFuzz)) {
					if (mValleyTime - mLastPeakTime > mTimeFuzz || mLastPeak - mValleyValue > mValueFuzz) {
						result = buildValueFalling();
					}
				}

				mFindPeak = false;
				mFindValley = true;

				mLastValleyTime = mValleyTime;
				mLastValley = mValleyValue;
				mValleyValue = value;
			}
		} else if (mFindValley) {
			if (value < mValleyValue) {
				mValleyValue = value;
			} else {
				mValleyTime = System.currentTimeMillis();

				double diff = getDiff();

				if (getDiff() > mValueFuzz || (diff > mValueFuzzMin && mValleyTime - mPeakTime > mTimeFuzz)) {
					if(mPeakTime - mLastValleyTime > mTimeFuzz || mPeakValue - mLastValley > mValueFuzz) {
						result = buildValueRising();
					}
				}

				mFindPeak = true;

				mLastPeakTime = mPeakTime;
				mLastPeak = mPeakValue;
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
				init(value);
			}
		} else {
			init(value);
			mInitialized = true;
		}

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

	public double putFreqValue(double value) {
		long time = System.currentTimeMillis();

		CavanPeakValleyValue result = putValue(value);
		if (result != null) {
			mTime = time;

			while (mValueList.size() > FREQ_COUNT) {
				mValueList.remove(0);
			}

			mValueList.add(result);

			int count = mValueList.size();
			if (count > 1) {
				setFreq(mValueList.get(0), time, count);
			}
		} else if (time - mTime < FREQ_TIMEOUT) {
			int count = mValueList.size();
			if (count > 1) {
				setFreq(mValueList.get(0), time, count);
			} else {
				setFreq(0);
			}
		} else {
			mValueList.clear();
			setFreq(0);
		}

		return mFreq;
	}
}