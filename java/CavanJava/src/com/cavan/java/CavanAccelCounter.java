package com.cavan.java;

public class CavanAccelCounter {

	private long mCount;
	private CavanSquareWaveCounter mCounterX;
	private CavanSquareWaveCounter mCounterY;
	private CavanSquareWaveCounter mCounterZ;
	private CavanSquareWaveCounter mCounterBest;

	public CavanAccelCounter(double fuzz, long timeMin, long timeMax) {
		mCounterX = new CavanSquareWaveCounter(fuzz, timeMin, timeMax);
		mCounterY = new CavanSquareWaveCounter(fuzz, timeMin, timeMax);
		mCounterZ = new CavanSquareWaveCounter(fuzz, timeMin, timeMax);

		mCounterBest = mCounterX;
	}

	public void setValueFuzz(double fuzz) {
		mCounterX.setValueFuzz(fuzz);
		mCounterY.setValueFuzz(fuzz);
		mCounterZ.setValueFuzz(fuzz);
	}

	public void setTimeMin(long time) {
		mCounterX.setTimeMin(time);
		mCounterY.setTimeMin(time);
		mCounterZ.setTimeMin(time);
	}

	public void setTimeMax(long time) {
		mCounterX.setOverTime(time);
		mCounterY.setOverTime(time);
		mCounterZ.setOverTime(time);
	}

	public CavanSquareWaveCounter getCounter() {
		return mCounterBest;
	}

	public CavanSquareWaveCounter getCounterX() {
		return mCounterX;
	}

	public CavanSquareWaveCounter getCounterY() {
		return mCounterY;
	}

	public CavanSquareWaveCounter getCounterZ() {
		return mCounterZ;
	}

	public double getFreq() {
		return mCounterBest.getFreq();
	}

	public double getValueRange() {
		return mCounterBest.getValueRange();
	}

	public boolean getValue() {
		return mCounterBest.getValue();
	}

	public long getCount() {
		return mCounterBest.getCount() + mCount;
	}

	private void setBestCounter(CavanSquareWaveCounter counter) {
		mCount += mCounterBest.getCount();
		mCounterBest = counter;
		counter.setCount(0);
	}

	public CavanSquareWaveCounter putValue(double x, double y, double z) {
		mCounterX.putFreqValue(x);
		mCounterY.putFreqValue(y);
		mCounterZ.putFreqValue(z);

		if (mCounterX.getValueRange() > mCounterY.getValueRange()) {
			if (mCounterX.getValueRange() > mCounterZ.getValueRange()) {
				setBestCounter(mCounterX);
			} else {
				setBestCounter(mCounterZ);
			}
		} else if (mCounterY.getValueRange() > mCounterZ.getValueRange()) {
			setBestCounter(mCounterY);
		} else {
			setBestCounter(mCounterZ);
		}

		return mCounterBest;
	}

	public CavanSquareWaveCounter putValue(float[] values) {
		return putValue(values[0], values[1], values[2]);
	}
}
