package com.cavan.java;

@SuppressWarnings("serial")
public class CavanSquareWaveGenerator extends CavanWaveArray {

	public final double DEFAULT_THRESHOLD = 0.25;

	protected long mTimeMin;
	protected double mValueFuzz;

	protected double mThresholdLow;
	protected double mThresholdHigh;

	protected double mValueRange;
	protected double mValueRangeAvg;

	protected double mThreshold;
	protected boolean mValue;

	public CavanSquareWaveGenerator(double fuzz, long timeMin, long timeMax) {
		super(timeMax);

		mValueFuzz = fuzz;
		mTimeMin = timeMin;

		setThreshold(DEFAULT_THRESHOLD);
	}

	public void setValueFuzz(double fuzz) {
		mValueFuzz = fuzz;
	}

	public void setTimeMin(long time) {
		mTimeMin = time;
	}

	public void setThreshold(double threshold) {
		if (threshold > 1) {
			threshold = 1;
		}

		if (mMinNode != null && mMaxNode != null) {
			updateThreshold();
		}
	}

	private void updateThreshold() {
		double min = mMinNode.getValue();
		double max = mMaxNode.getValue();

		mValueRange = max - min;
		mValueRangeAvg = (mValueRange + mValueRangeAvg * 3) / 4;

		if (mLastNode.getTime() - get(0).getTime() > mTimeMin) {
			updateThreshold(min, max);
		} else {
			mThresholdHigh = min + mValueFuzz;
			mThresholdLow = max - mValueFuzz;
		}
	}

	protected void updateThreshold(double min, double max) {
		double fuzz = mValueRange * mThreshold;
		if (fuzz < mValueFuzz) {
			fuzz = mValueFuzz;
		}

		mThresholdHigh = (min + max + fuzz) / 2;
		mThresholdLow = mThresholdHigh - fuzz;
	}

	@Override
	protected void updateExtremeNode() {
		super.updateExtremeNode();
		updateThreshold();
	}

	@Override
	protected void setMaxNode(CavanTimedNode<Double> node) {
		super.setMaxNode(node);
		updateThreshold();
	}

	@Override
	protected void setMinNode(CavanTimedNode<Double> node) {
		super.setMinNode(node);
		updateThreshold();
	}

	public boolean getValue() {
		return mValue;
	}

	public void setValue(boolean value) {
		mValue = value;
	}

	public double getValueRange() {
		return mValueRange;
	}

	public double getValueRangeAvg() {
		return mValueRangeAvg;
	}

	public boolean putValue(double value) {
		addWaveValue(value);

		if (value < mThresholdLow) {
			mValue = false;
		} else if (value > mThresholdHigh) {
			mValue = true;
		}

		return mValue;
	}
}
