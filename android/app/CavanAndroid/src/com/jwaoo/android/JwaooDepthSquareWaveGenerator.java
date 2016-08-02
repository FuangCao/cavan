package com.jwaoo.android;

import com.cavan.java.CavanSquareWaveGenerator;


public class JwaooDepthSquareWaveGenerator extends CavanSquareWaveGenerator {

	private double mValueMax;
	private double mValueMin;

	public JwaooDepthSquareWaveGenerator(double fuzz, long timeMin, long timeMax) {
		super(fuzz, timeMin, timeMax);
	}

	public double getMaxValue() {
		return mValueMax;
	}

	public double getMinValue() {
		return mValueMin;
	}

	@Override
	protected void updateThreshold(double min, double max) {
		if (mValueDiff > mValueFuzz) {
			mValueMax = max;
			mValueMin = min;
		} else if (max > mValueMax) {
			mValueMax = max;
			min = mValueMin;
		} else if (min < mValueMin) {
			mValueMin = min;
			max = mValueMax;
		} else {
			return;
		}

		mThresholdHigh = (max + min) / 2;
		mThresholdLow = min + (max - min) / 4;
	}
}