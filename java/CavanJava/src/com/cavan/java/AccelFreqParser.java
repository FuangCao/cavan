package com.cavan.java;

public class AccelFreqParser {

	private int mFreq;

	private CavanPeakValleyFinder mFinderX;
	private CavanPeakValleyFinder mFinderY;
	private CavanPeakValleyFinder mFinderZ;
	private CavanPeakValleyFinder mFinderBest;

	protected void onFreqChanged(int freq) {
		// CavanAndroid.logE("freq = " + freq);
	}

	public AccelFreqParser(long timeFuzz, double valueFuzz) {
		mFinderX = new CavanPeakValleyFinder(timeFuzz, valueFuzz);
		mFinderY = new CavanPeakValleyFinder(timeFuzz, valueFuzz);
		mFinderZ = new CavanPeakValleyFinder(timeFuzz, valueFuzz);

		mFinderBest = mFinderX;
	}

	public int getFreq() {
		return mFinderBest.getFreq();
	}

	private void updateFreq(int freq) {
		if (freq > 0 && mFreq > 0) {
			freq = (mFreq + freq) / 2;
		}

		if (mFreq != freq) {
			mFreq = freq;
			onFreqChanged(freq);
		}
	}

	public void putValue(double x, double y, double z) {
		mFinderX.putFreqValue(x);
		mFinderY.putFreqValue(y);
		mFinderZ.putFreqValue(z);

		if (mFinderX.getAvgDiff() > mFinderY.getAvgDiff()) {
			if (mFinderX.getAvgDiff() > mFinderZ.getAvgDiff()) {
				mFinderBest = mFinderX;
			} else {
				mFinderBest = mFinderZ;
			}
		} else if (mFinderY.getAvgDiff() > mFinderZ.getAvgDiff()) {
			mFinderBest = mFinderY;
		} else {
			mFinderBest = mFinderZ;
		}

		updateFreq(mFinderBest.getFreq());
	}

	public void putValue(AccelDataCache cache) {
		putValue(cache.getCoorX(), cache.getCoorY(), cache.getCoorZ());
	}
}
