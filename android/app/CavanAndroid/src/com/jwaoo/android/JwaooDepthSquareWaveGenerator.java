package com.jwaoo.android;

import com.cavan.java.CavanCountedArray;
import com.cavan.java.CavanSquareWaveGenerator;


@SuppressWarnings("serial")
public class JwaooDepthSquareWaveGenerator extends CavanSquareWaveGenerator {

	private boolean mHasPredictedMin;
	private boolean mHasPredictedMax;
	private double mPredictedMin = JwaooToySensor.CAPACITY_MAX;
	private double mPredictedMax = JwaooToySensor.CAPACITY_MIN;

	private double mCapacity;
	private double mCapacityRange;
	private double mCapacityMin = JwaooToySensor.CAPACITY_MIN;
	private double mCapacityMax = JwaooToySensor.CAPACITY_MAX;
	private CavanCountedArray<Integer> mCountedArrayMin = new CavanCountedArray<Integer>(500);
	private CavanCountedArray<Integer> mCountedArrayMax = new CavanCountedArray<Integer>(500);

	public JwaooDepthSquareWaveGenerator(double fuzz, long timeMin, long timeMax) {
		super(fuzz, timeMin, timeMax);
	}

	public void savePredictedMin() {
		mHasPredictedMin = true;
		mPredictedMin = mCapacity;
	}

	public void savePredictMax() {
		mHasPredictedMax = true;
		mPredictedMax = mCapacity;
	}

	public double getPredictedRange() {
		if (mHasPredictedMin && mHasPredictedMax) {
			return mPredictedMax - mPredictedMin;
		}

		return 0.0;
	}

	public void setPredictedRange(double range) {
		if (mHasPredictedMax) {
			if (!mHasPredictedMin) {
				mPredictedMin = mPredictedMax - range;
			}
		} else if (mHasPredictedMin) {
			mPredictedMax = mPredictedMin + range;
		}
	}

	public double getPredictedValue() {
		if (mCapacity >= mPredictedMax) {
			return 1.0;
		} else if (mCapacity <= mPredictedMin) {
			return 0.0;
		} else {
			return (mCapacity - mPredictedMin) / (mPredictedMax - mPredictedMin);
		}
	}

	private boolean isValidMinMax(double min, double max) {
		if (max - min < mValueFuzz) {
			return false;
		}

		if (mCapacityMax - min < mValueFuzz) {
			return false;
		}

		return true;
	}

	@Override
	protected void updateThreshold(double min, double max) {
		if (isValidMinMax(min, max)) {
			mCapacityMin = mCountedArrayMin.putCountedValue((int) min);
			mCapacityMax = mCountedArrayMax.putCountedValue((int) max);
		} else if (min < mCapacityMin) {
			mCapacityMin = min;
		} else {
			return;
		}

		if (mCapacityMin > mPredictedMin) {
			mCapacityMin = mPredictedMin;
		}

		if (mCapacityMax < mPredictedMax) {
			mCapacityMax = mPredictedMax;
		}

		mCapacityRange = mCapacityMax - mCapacityMin;
		mThresholdLow = mCapacityMin + mCapacityRange / 4;
		mThresholdHigh = (mCapacityMax + mCapacityMin) / 2; // mCapacityMax - mCapacityRange / 4;
	}

	@Override
	public boolean putValue(double value) {
		mCapacity = value;

		boolean result = super.putValue(value);

		double min = getMinValue();
		double max = getMaxValue();

		if (isValidMinMax(min, max)) {
			mCountedArrayMin.addCountedValue((int) min);
			mCountedArrayMax.addCountedValue((int) max);
		}

		return result;
	}
}
