package com.cavan.java;

public class AccelFreqParser {

	private int mFreq;

	private CavanPeakValleyFinder mFinderX;
	private CavanPeakValleyFinder mFinderY;
	private CavanPeakValleyFinder mFinderZ;
	private CavanPeakValleyFinder mFinderBetter;

	protected void onFreqChanged(int freq) {
		// CavanAndroid.logE("freq = " + freq);
	}

	public AccelFreqParser(long timeFuzz, double valueFuzz) {
		mFinderX = new CavanPeakValleyFinder(timeFuzz, valueFuzz);
		mFinderY = new CavanPeakValleyFinder(timeFuzz, valueFuzz);
		mFinderZ = new CavanPeakValleyFinder(timeFuzz, valueFuzz);

		mFinderBetter = mFinderX;
	}

	public int getFreq() {
		return mFinderBetter.getFreq();
	}

	private void setFreq(int freq) {
		freq = (mFreq + freq) / 2;
		if (mFreq != freq) {
			mFreq = freq;
			onFreqChanged(freq);
		}
	}

	public int putValue(double x, double y, double z) {
		mFinderX.putFreqValue(x);
		mFinderY.putFreqValue(y);
		mFinderZ.putFreqValue(z);

		if (mFinderX.getAvgDiff() > mFinderY.getAvgDiff()) {
			if (mFinderX.getAvgDiff() > mFinderZ.getAvgDiff()) {
				mFinderBetter = mFinderX;
			} else {
				mFinderBetter = mFinderZ;
			}
		} else if (mFinderY.getAvgDiff() > mFinderZ.getAvgDiff()) {
			mFinderBetter = mFinderY;
		} else {
			mFinderBetter = mFinderZ;
		}

		setFreq(mFinderBetter.getFreq());

		return mFreq;
	}

	public int putValue(AccelDataCache cache) {
		return putValue(cache.getCoorX(), cache.getCoorY(), cache.getCoorZ());
	}
}
