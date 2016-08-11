package com.cavan.java;

@SuppressWarnings("serial")
public class CavanSquareWaveCounter extends CavanSquareWaveGenerator {

	private long mCount;
	private boolean mLastValue;
	private CavanTimedArray<Double> mFreqNodes;

	public CavanSquareWaveCounter(double fuzz, long timeMin, long overtimeValue, long overtimeFreq) {
		super(fuzz, timeMin, overtimeValue);
		mFreqNodes = new CavanTimedArray<Double>(overtimeFreq);
	}

	public long getCount() {
		return mCount;
	}

	public void setCount(long count) {
		mCount = count;
	}

	public void setFreqOverTime(long time) {
		mFreqNodes.setOverTime(time);
	}

	@Override
	public double getFreq() {
		return mFreqNodes.getFreq();
	}

	@Override
	public long getCycle() {
		return mFreqNodes.getCycle();
	}

	@Override
	protected double updateFreq() {
		mFreq = mFreqNodes.updateFreq();
		return mFreq;
	}

	public double putFreqValue(double value) {
		boolean result = putValue(value);

		if (result && mLastValue == false) {
			mFreqNodes.addTimedNode(mLastNode);
			updateFreq();
			mCount++;
		} else if (mFreqNodes.removeOvertimeNodes() > 0) {
			updateFreq();
		}

		mLastValue = result;

		return mFreq;
	}
}
