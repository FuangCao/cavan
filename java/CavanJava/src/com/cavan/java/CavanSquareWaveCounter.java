package com.cavan.java;

@SuppressWarnings("serial")
public class CavanSquareWaveCounter extends CavanSquareWaveGenerator {

	private long mCount;
	private boolean mLastValue;
	private CavanTimedList<Double> mTimedList;

	public CavanSquareWaveCounter(double fuzz, long timeMin, long timeMax) {
		super(fuzz, timeMin, timeMax);
		mTimedList = new CavanTimedList<Double>(timeMax);
	}

	public long getCount() {
		return mCount;
	}

	public void setCount(long count) {
		mCount = count;
	}

	@Override
	public void setOverTime(long time) {
		mTimedList.setOverTime(time);
		super.setOverTime(time);
	}

	@Override
	protected double updateFreq() {
		mFreq = mTimedList.updateFreq();
		return mFreq;
	}

	public double putFreqValue(double value) {
		boolean result = putValue(value);

		if (result && mLastValue == false) {
			mTimedList.addTimedNode(mLastNode);
			updateFreq();
			mCount++;
		} else if (mTimedList.removeOvertimeNodes() > 0) {
			updateFreq();
		}

		mLastValue = result;

		return mFreq;
	}
}
