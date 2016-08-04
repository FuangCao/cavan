package com.jwaoo.android;

import com.cavan.java.CavanPeakValleyFinder;
import com.cavan.java.CavanSquareWaveGenerator;


@SuppressWarnings("serial")
public class JwaooDepthSquareWaveGenerator extends CavanSquareWaveGenerator {

	private double mMaxPeakValue = JwaooToySensor.CAPACITY_MIN;
	private double mMinPeakValue = JwaooToySensor.CAPACITY_MIN;
	private CavanPeakValleyFinder mFinderMax = new CavanPeakValleyFinder();
	private CavanPeakValleyFinder mFinderMin = new CavanPeakValleyFinder();

	public JwaooDepthSquareWaveGenerator(double fuzz, long timeMin, long timeMax) {
		super(fuzz, timeMin, timeMax);
	}

	@Override
	protected void updateThreshold(double min, double max) {
		if (mValueDiff > mValueFuzz) {
			if (mFinderMax.putValue(max)) {
				mMaxPeakValue = mFinderMax.getPeakValue();
			}

			if (max < mMaxPeakValue) {
				max = mMaxPeakValue;;
			}

			if (mFinderMin.putValue(min)) {
				mMinPeakValue = mFinderMin.getPeakValue();
			}

			if (min < mMinPeakValue) {
				min = mMinPeakValue;
			}

			mThresholdHigh = (max + min) / 2;
			mThresholdLow = min + (max - min) / 4;
		}
	}
}