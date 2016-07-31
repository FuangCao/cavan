package com.cavan.java;

import java.util.ArrayList;
import java.util.List;

public class CavanSquareWaveGenerator {

	public final double DEFAULT_THRESHOLD = 0.35;

	protected double mFuzz;
	protected long mTimeMin;
	protected long mTimeMax;

	protected double mValueMin;
	protected double mValueMax;
	protected double mValueDiff;

	private double mThreshold;
	private CavanWaveValue mNodeMin;
	private CavanWaveValue mNodeMax;

	private boolean mValue;
	private boolean mNeedFind = true;

	private List<CavanWaveValue> mValues = new ArrayList<CavanWaveValue>();

	public CavanSquareWaveGenerator(double fuzz, long timeMin, long timeMax) {
		mFuzz = fuzz;
		mTimeMin = timeMin;
		mTimeMax = timeMax;

		setThreshold(DEFAULT_THRESHOLD);
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

	public void updateThreshold() {
		double min = mNodeMin.getValue();
		double max = mNodeMax.getValue();

		mValueDiff = max - min;
		if (mValueDiff < mFuzz) {
			mValueMin = max + mFuzz;
		} else {
			double threshold = mValueDiff * mThreshold;

			mValueMax = max - threshold;
			mValueMin = min + threshold;
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

	public double getDiff() {
		return mValueDiff;
	}

	public boolean putValue(double value) {
		long time = System.currentTimeMillis();

		while (true) {
			CavanWaveValue node;

			try {
				node = mValues.get(0);
			} catch (Exception e) {
				break;
			}

			if (time - node.getTime() < mTimeMax) {
				break;
			}

			if (node == mNodeMin || node == mNodeMax) {
				mNeedFind = true;
			}

			mValues.remove(0);
		}

		CavanWaveValue newNode = new CavanWaveValue(value, time);
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

		if (time - mValues.get(0).getTime() < mTimeMin) {
			return false;
		}

		if (value < mValueMin) {
			mValue = false;
		} else if (value > mValueMax) {
			mValue = true;
		}

		return mValue;
	}
}
