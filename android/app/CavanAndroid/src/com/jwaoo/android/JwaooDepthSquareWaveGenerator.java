package com.jwaoo.android;

import com.cavan.java.CavanCountedArray;
import com.cavan.java.CavanSquareWaveGenerator;


@SuppressWarnings("serial")
public class JwaooDepthSquareWaveGenerator extends CavanSquareWaveGenerator {

	private double mMinValue = JwaooToySensor.CAPACITY_MIN;
	private double mMaxValue = JwaooToySensor.CAPACITY_MAX;
	private CavanCountedArray<Integer> mCountedArrayMin = new CavanCountedArray<Integer>(10, 500);
	private CavanCountedArray<Integer> mCountedArrayMax = new CavanCountedArray<Integer>(10, 500);

	public JwaooDepthSquareWaveGenerator(double fuzz, long timeMin, long timeMax) {
		super(fuzz, timeMin, timeMax);
	}

	@Override
	protected void updateThreshold(double min, double max) {
		if (mValueRange > mValueFuzz) {
			mMinValue = mCountedArrayMin.putCountedValue((int) min);
			mMaxValue = mCountedArrayMax.putCountedValue((int) max);
		} else if (min < mMinValue) {
			mMinValue = min;
		} else if (max > mMaxValue) {
			mMaxValue = max;
		} else {
			return;
		}

		double range = mMaxValue - mMinValue;
		mThresholdLow = mMinValue + range / 4;
		mThresholdHigh = mMaxValue - range / 4;
	}

	@Override
	public boolean putValue(double value) {
		boolean result = super.putValue(value);

		double min = getMinValue();
		double max = getMaxValue();

		if (max - min > mValueFuzz) {
			// CavanAndroid.logE("min = " + min + ", max = " + max);
			mCountedArrayMin.addCountedValue((int) min);
			mCountedArrayMax.addCountedValue((int) max);
		}

		return result;
	}
}