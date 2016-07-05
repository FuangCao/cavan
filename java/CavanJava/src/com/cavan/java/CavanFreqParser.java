package com.cavan.java;

import java.util.ArrayList;
import java.util.List;

public class CavanFreqParser {
	private double mFuzz;
	private double mFreq;
	private double mAvgDiff;
	private long mLastTime;
	private CavanPeakValleyValue mValue;
	private CavanPeakValleyFinder mFinder = new CavanPeakValleyFinder();
	private List<Long> mListTime = new ArrayList<Long>();

	protected void onFreqChanged(double freq) {

	}

	public CavanFreqParser(double fuzz) {
		mFuzz = fuzz;
	}

	public double getFreq() {
		return mFreq;
	}

	public void setFreq(double freq) {
		if (mFreq != freq) {
			mFreq = freq;
			onFreqChanged(mFreq);
		}
	}

	public double getAvgDiff() {
		return mAvgDiff;
	}

	public CavanPeakValleyValue putValue(double value) {
		long time = System.currentTimeMillis();

		CavanPeakValleyValue pvValue = mFinder.putValue(value);
		if (pvValue == null || pvValue.getDiff() < mFuzz) {
			if (mFreq > 0 && time - mLastTime > 2000) {
				mListTime.clear();
				mAvgDiff = 0;
				setFreq(0);
			}

			return null;
		}

		mLastTime = time;

		CavanPeakValleyValue result = null;

		if (mValue != null) {
			if (mValue.getPeakValue() - pvValue.getValleyValue() < mFuzz || pvValue.getPeakValue() - mValue.getValleyValue() < mFuzz) {
				mValue.extend(pvValue);
			} else {
				result = mValue;
				mValue = pvValue;

				mListTime.add(time);

				if (mListTime.size() > 1) {
					while (mListTime.size() > 10) {
						mListTime.remove(0);
					}

					long startTime = mListTime.get(0);
					long lastTime = mListTime.get(mListTime.size() - 1);
					long diffTime = lastTime - startTime;

					setFreq((mListTime.size() - 1) * 60000 / diffTime / 2);
				}

				mAvgDiff = (mAvgDiff + result.getDiff()) / 2;
			}
		} else {
			mValue = pvValue;
		}

		return result;
	}
}
