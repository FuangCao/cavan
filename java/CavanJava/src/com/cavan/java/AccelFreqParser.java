package com.cavan.java;

public class AccelFreqParser {
	private double mCoorX;
	private double mCoorY;
	private double mCoorZ;

	private CavanFreqParser mParserX;
	private CavanFreqParser mParserY;
	private CavanFreqParser mParserZ;

	public AccelFreqParser(double fuzz) {
		mParserX = new CavanFreqParser(fuzz);
		mParserY = new CavanFreqParser(fuzz);
		mParserZ = new CavanFreqParser(fuzz);
	}

	public CavanFreqParser putValue(AccelDataCache cache) {
		mCoorX = (mCoorX + cache.getCoorX()) / 2;
		mCoorY = (mCoorY + cache.getCoorY()) / 2;
		mCoorZ = (mCoorZ + cache.getCoorZ()) / 2;

		mParserX.putValue(mCoorX);
		mParserY.putValue(mCoorY);
		mParserZ.putValue(mCoorZ);

		if (mParserX.getAvgDiff() > mParserY.getAvgDiff()) {
			if (mParserX.getAvgDiff() > mParserZ.getAvgDiff()) {
				return mParserX;
			} else {
				return mParserZ;
			}
		} else if (mParserY.getAvgDiff() > mParserZ.getAvgDiff()) {
			return mParserY;
		} else {
			return mParserZ;
		}
	}
}