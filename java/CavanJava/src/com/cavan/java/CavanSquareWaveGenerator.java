package com.cavan.java;

import java.util.ArrayList;
import java.util.List;

public class CavanSquareWaveGenerator {

	public final double DEFAULT_THRESHOLD = 0.35;

	protected long mTimeMin;
	protected long mTimeMax;
	protected double mValueFuzz;

	protected double mThresholdLow;
	protected double mThresholdHigh;

	protected double mValueDiff;
	protected double mValueAvgDiff;

	protected double mThreshold;
	protected CavanWaveValue mNodeMin;
	protected CavanWaveValue mNodeMax;

	protected long mTime;
	protected boolean mValue;
	protected boolean mNeedFind = true;

	private List<CavanWaveValue> mValues = new ArrayList<CavanWaveValue>();

	public CavanSquareWaveGenerator(double fuzz, long timeMin, long timeMax) {
		mValueFuzz = fuzz;
		mTimeMin = timeMin;
		mTimeMax = timeMax;

		setThreshold(DEFAULT_THRESHOLD);
	}

	public void setValueFuzz(double fuzz) {
		mValueFuzz = fuzz;
	}

	public void setTimeMin(long time) {
		mTimeMin = time;
	}

	public void setTimeMax(long time) {
		mTimeMax = time;
	}

	public void setThreshold(double threshold) {
		if (threshold > 1.0) {
			threshold = 1.0;
		}

		mThreshold = (1.0 - threshold) / 2;

		if (mNodeMin != null && mNodeMax != null) {
			updateThreshold();
		}
	}

	private void updateThreshold() {
		double min = mNodeMin.getValue();
		double max = mNodeMax.getValue();

		mValueDiff = max - min;
		mValueAvgDiff = (mValueDiff + mValueAvgDiff * 3) / 4;

		if (mTime - mValues.get(0).getTime() > mTimeMin) {
			updateThreshold(min, max);
		} else {
			mThresholdHigh = Double.MAX_VALUE;
			mThresholdLow = Double.MIN_VALUE;
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

	public void setMinNode(CavanWaveValue node) {
		mNodeMin = node;
		updateThreshold();
	}

	public void setMaxNode(CavanWaveValue node) {
		mNodeMax = node;
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

	protected void addNode(double value) {
		while (true) {
			CavanWaveValue node;

			try {
				node = mValues.get(0);
			} catch (Exception e) {
				break;
			}

			if (mTime - node.getTime() < mTimeMax) {
				break;
			}

			if (node == mNodeMin || node == mNodeMax) {
				mNeedFind = true;
			}

			mValues.remove(0);
		}

		CavanWaveValue newNode = new CavanWaveValue(value, mTime);
		mValues.add(newNode);

		if (mNeedFind) {
			mNodeMin = mNodeMax = mValues.get(0);

			for (int i = mValues.size() - 1; i > 0; i--) {
				CavanWaveValue node = mValues.get(i);
				if (node.getValue() < mNodeMin.getValue()) {
					mNodeMin = node;
				} else if (node.getValue() > mNodeMax.getValue()) {
					mNodeMax = node;
				}
			}

			updateThreshold();
		} else if (value < mNodeMin.getValue()) {
			setMinNode(newNode);
		} else if (value > mNodeMax.getValue()) {
			setMaxNode(newNode);
		}
	}

	public boolean putValue(double value) {
		mTime = System.currentTimeMillis();

		addNode(value);

		if (value < mThresholdLow) {
			mValue = false;
		} else if (value > mThresholdHigh) {
			mValue = true;
		}

		return mValue;
	}
}
