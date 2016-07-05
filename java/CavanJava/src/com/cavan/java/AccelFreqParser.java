package com.cavan.java;

public class AccelFreqParser {

	private CavanFreqParser mParserX;
	private CavanFreqParser mParserY;
	private CavanFreqParser mParserZ;

	public AccelFreqParser(double fuzz) {
		mParserX = new CavanFreqParser(fuzz);
		mParserY = new CavanFreqParser(fuzz);
		mParserZ = new CavanFreqParser(fuzz);
	}

	public CavanFreqParser putValue(AccelDataCache cache) {
		mParserX.putValue(cache.getCoorX());
		mParserY.putValue(cache.getCoorY());
		mParserZ.putValue(cache.getCoorZ());

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