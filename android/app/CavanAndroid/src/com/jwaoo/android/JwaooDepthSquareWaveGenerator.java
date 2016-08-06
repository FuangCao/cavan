package com.jwaoo.android;

import com.cavan.java.CavanCountedArray;
import com.cavan.java.CavanSquareWaveGenerator;


@SuppressWarnings("serial")
public class JwaooDepthSquareWaveGenerator extends CavanSquareWaveGenerator {

	private double mDepth;
	private double mCapacityRange;
	private double mCapacityMin = JwaooToySensor.CAPACITY_MIN;
	private double mCapacityMax = JwaooToySensor.CAPACITY_MAX;
	private CavanCountedArray<Integer> mCountedArrayMin = new CavanCountedArray<Integer>(10, 500);
	private CavanCountedArray<Integer> mCountedArrayMax = new CavanCountedArray<Integer>(10, 500);

	public JwaooDepthSquareWaveGenerator(double fuzz, long timeMin, long timeMax) {
		super(fuzz, timeMin, timeMax);
	}

	public double getDepth() {
		return mDepth;
	}

	@Override
	protected void updateThreshold(double min, double max) {
		if (mValueRange > mValueFuzz) {
			mCapacityMin = mCountedArrayMin.putCountedValue((int) min);
			mCapacityMax = mCountedArrayMax.putCountedValue((int) max);
		} else if (min < mCapacityMin) {
			mCapacityMin = min;
		} else if (max > mCapacityMax) {
			mCapacityMax = max;
		} else {
			return;
		}

		mCapacityRange = mCapacityMax - mCapacityMin;
		mThresholdLow = mCapacityMin + mCapacityRange / 4;
		mThresholdHigh = mCapacityMax - mCapacityRange / 4;
	}

	@Override
	public boolean putValue(double value) {
		boolean result = super.putValue(value);

		double min = getMinValue();
		double max = getMaxValue();

		if (max - min > mValueFuzz) {
			mCountedArrayMin.addCountedValue((int) min);
			mCountedArrayMax.addCountedValue((int) max);
		}

		if (value < mCapacityMin) {
			mDepth = 0;
		} else if (value > mCapacityMax) {
			mDepth = 1;
		} else {
			try {
				mDepth = (value - mCapacityMin) / mCapacityRange;
			} catch (Exception e) {
				mDepth = 0;
			}
		}

		return result;
	}
}