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

	public CavanSquareWaveCounter getCounter() {
		return mCounterBest;
	}

	public double getFreq() {
		return mCounterBest.getFreq();
	}

	public double getDiff() {
		return mCounterBest.getDiff();
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

		if (mCounterX.getDiff() > mCounterY.getDiff()) {
			if (mCounterX.getDiff() > mCounterZ.getDiff()) {
				setBestCounter(mCounterX);
			} else {
				setBestCounter(mCounterZ);
			}
		} else if (mCounterY.getDiff() > mCounterZ.getDiff()) {
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
