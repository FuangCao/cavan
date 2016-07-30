package com.cavan.java;

import java.util.ArrayList;
import java.util.List;

public class CavanSquareWaveCounter extends CavanSquareWaveGenerator {

	private long mCount;
	private double mFreq;
	private boolean mLastValue;
	private List<Long> mTimes = new ArrayList<Long>();

	public CavanSquareWaveCounter(double fuzz, long timeMin, long timeMax) {
		super(fuzz, timeMin, timeMax);
	}

	public long getCount() {
		return mCount;
	}

	public void setCount(long count) {
		mCount = count;
	}

	public double getFreq() {
		return mFreq;
	}

	public double putFreqValue(double value) {
		long time = System.currentTimeMillis();

		while (true) {
			try {
				if (time - mTimes.get(0) < mTimeMax) {
					break;
				}
			} catch (Exception e) {
				break;
			}

			mTimes.remove(0);
		}

		if (putValue(value) && mLastValue == false) {
			mTimes.add(time);
			mCount++;
		}

		mLastValue = getValue();

		int count = mTimes.size();
		if (count > 1) {
			mFreq = ((double) (count - 1) * 1000) / (mTimes.get(count - 1) - mTimes.get(0));
		} else {
			mFreq = 0;
		}

		return mFreq;
	}
}
