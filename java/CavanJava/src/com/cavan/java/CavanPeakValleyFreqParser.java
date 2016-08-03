package com.cavan.java;

import java.util.ArrayList;


public class CavanPeakValleyFreqParser extends CavanPeakValleyFilter {

	public static final long FREQ_OVER_TIME = 3000;

	private double mFreq;
	private CavanRangeValue mRange;

	public CavanPeakValleyFreqParser(double fuzz) {
		super(fuzz, FREQ_OVER_TIME);
	}

	public double getFreq() {
		return mFreq;
	}

	public double getRange() {
		if (mRange == null) {
			return 0;
		}

		return mRange.getRange();
	}

	private double calculateFreq(ArrayList<CavanPeakValleyValue> nodes) {
		mRange = getExtremeValue(nodes);
		if (mRange == null) {
			return 0;
		}

		int index = 0;
		double range = mRange.getRange() / 2;

		while (index < nodes.size()) {
			if (nodes.get(index).getRange() < range) {
				nodes.remove(index);
			} else {
				index++;
			}
		}

		index = nodes.size() - 1;
		if (index > 0) {
			return 1000.0 * index / (nodes.get(0).getTime() - nodes.get(index).getTime());
		}

		return 0;
	}

	public double putFreqValueRise(double value) {
		if (putValueRise(value)) {
			mFreq = calculateFreq(mRiseNodes);
		}

		return mFreq;
	}

	public double putFreqValueFall(double value) {
		if (putValueFall(value)) {
			mFreq = calculateFreq(mFallNodes);
		}

		return mFreq;
	}
}
