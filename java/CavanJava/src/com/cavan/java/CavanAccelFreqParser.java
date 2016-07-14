package com.cavan.java;

public class CavanAccelFreqParser {

	private int mFreq;

	private CavanPeakValleyFinder mFinderX;
	private CavanPeakValleyFinder mFinderY;
	private CavanPeakValleyFinder mFinderZ;
	private CavanPeakValleyFinder mFinderBest;

	protected void onFreqChanged(int freq) {}

	public CavanAccelFreqParser(long timeFuzz, double valueFuzz) {
		mFinderX = new CavanPeakValleyFinder(timeFuzz, valueFuzz);
		mFinderY = new CavanPeakValleyFinder(timeFuzz, valueFuzz);
		mFinderZ = new CavanPeakValleyFinder(timeFuzz, valueFuzz);

		mFinderBest = mFinderX;
	}

	public int getFreq() {
		return mFinderBest.getFreq();
	}

	public void setValueFuzz(double fuzz) {
		mFinderX.setValueFuzz(fuzz);
		mFinderY.setValueFuzz(fuzz);
		mFinderZ.setValueFuzz(fuzz);
	}

	public void setTimeFuzz(long fuzz) {
		mFinderX.setTimeFuzz(fuzz);
		mFinderY.setTimeFuzz(fuzz);
		mFinderZ.setTimeFuzz(fuzz);
	}

	public CavanPeakValleyFinder getFinder() {
		return mFinderBest;
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

	public void putData(CavanAccelSensor sensor) {
		putValue(sensor.getAxisX(), sensor.getAxisY(), sensor.getAxisZ());
	}
}
