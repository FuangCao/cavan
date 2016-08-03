package com.cavan.java;

public class CavanPeakVallyAccelFreqParser {

	private CavanPeakValleyFreqParser mParserX;
	private CavanPeakValleyFreqParser mParserY;
	private CavanPeakValleyFreqParser mParserZ;
	private CavanPeakValleyFreqParser mParserBest;

	public CavanPeakVallyAccelFreqParser(double valueFuzz) {
		mParserX = new CavanPeakValleyFreqParser(valueFuzz);
		mParserY = new CavanPeakValleyFreqParser(valueFuzz);
		mParserZ = new CavanPeakValleyFreqParser(valueFuzz);

		mParserBest = mParserX;
	}

	public double getFreq() {
		return mParserBest.getFreq();
	}

	public void setValueFuzz(double fuzz) {
		mParserX.setValueFuzz(fuzz);
		mParserY.setValueFuzz(fuzz);
		mParserZ.setValueFuzz(fuzz);
	}

	public CavanPeakValleyFreqParser getParser() {
		return mParserBest;
	}

	public CavanPeakValleyFreqParser putValue(double x, double y, double z) {
		mParserX.putValue(x);
		mParserY.putValue(y);
		mParserZ.putValue(z);

		if (mParserX.getRange() > mParserY.getRange()) {
			if (mParserX.getRange() > mParserZ.getRange()) {
				mParserBest = mParserX;
			} else {
				mParserBest = mParserZ;
			}
		} else if (mParserY.getRange() > mParserZ.getRange()) {
			mParserBest = mParserY;
		} else {
			mParserBest = mParserZ;
		}

		return mParserBest;
	}

	public void putData(CavanAccelSensor sensor) {
		putValue(sensor.getAxisX(), sensor.getAxisY(), sensor.getAxisZ());
	}
}
