package com.cavan.java;



@SuppressWarnings("serial")
public class CavanSquareWaveGenerator extends CavanWaveList {

	public final double DEFAULT_THRESHOLD = 0.35;

	protected long mTimeMin;
	protected double mValueFuzz;

	protected double mThresholdLow;
	protected double mThresholdHigh;

	protected double mValueDiff;
	protected double mValueAvgDiff;

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

	public void setTimeMax(long time) {
		setOverTime(time);
	}

	public void setThreshold(double threshold) {
		if (threshold > 1.0) {
			threshold = 1.0;
		}

		mThreshold = (1.0 - threshold) / 2;

		if (mMinNode != null && mMaxNode != null) {
			updateThreshold();
		}
	}

	private void updateThreshold() {
		double min = mMinNode.getValue();
		double max = mMaxNode.getValue();

		mValueDiff = max - min;
		mValueAvgDiff = (mValueDiff + mValueAvgDiff * 3) / 4;

		if (mTime - get(0).getTime() > mTimeMin) {
			updateThreshold(min, max);
		} else {
			mThresholdHigh = min + mValueFuzz;
			mThresholdLow = max - mValueFuzz;
		}
	}

	protected void updateThreshold(double min, double max) {
		if (mValueDiff > mValueFuzz) {
			double threshold = mValueDiff * mThreshold;

			mThresholdHigh = max - threshold;
			mThresholdLow = min + threshold;
		} else {
			mThresholdHigh = min + mValueFuzz;
			mThresholdLow = max - mValueFuzz;
		}
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

	public double getDiff() {
		return mValueDiff;
	}

	public double getAvgDiff() {
		return mValueAvgDiff;
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
