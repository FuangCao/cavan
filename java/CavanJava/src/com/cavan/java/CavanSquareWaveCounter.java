package com.cavan.java;


@SuppressWarnings("serial")
public class CavanSquareWaveCounter extends CavanSquareWaveGenerator {

	private long mCount;
	private double mFreq;
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

	public double getFreq() {
		return mFreq;
	}

	@Override
	public void setOverTime(long time) {
		mTimedList.setOverTime(time);
		super.setOverTime(time);
	}

	public double putFreqValue(double value) {
		if (putValue(value) && mLastValue == false) {
			mTimedList.addTimedNode(mLastNode);
			mFreq = mTimedList.calculateFreq();
			mCount++;
		}

		mLastValue = getValue();

		return mFreq;
	}
}
