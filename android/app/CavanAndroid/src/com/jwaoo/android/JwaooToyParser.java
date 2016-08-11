package com.jwaoo.android;

import com.cavan.java.CavanSquareWaveCounter;

public class JwaooToyParser {

	private double mFreq;
	private double mDepth;

	private CavanSquareWaveCounter mCounterAccel;
	private JwaooDepthDecoder mDecoder;

	public JwaooToyParser(double accelFuzz, double depthFuzz, long timeMin, long overtimeValue, long overtimeFreq) {
		mCounterAccel = new CavanSquareWaveCounter(accelFuzz, timeMin, overtimeValue, overtimeFreq);
		mDecoder = new JwaooDepthDecoder(depthFuzz);
	}

	public double getFreq() {
		return mFreq;
	}

	public double getDepth() {
		return mDepth;
	}

	public void putData(JwaooToySensor sensor) {
		mDepth = mDecoder.putCapacityValue(sensor.getCapacitys());

		mCounterAccel.putFreqValue(sensor.getAxisX());
		if (mCounterAccel.getValueRangeAvg() > 2.0) {
			mFreq = mCounterAccel.getFreq();
		} else {
			mFreq = mDecoder.getFreq();
		}
	}

	public void setAccelFuzz(double fuzz) {
		mCounterAccel.setValueFuzz(fuzz);
	}

	public void setDepthFuzz(double fuzz) {
		mDecoder.setValueFuzz(fuzz);
	}

	public void setTimeMin(long time) {
		mCounterAccel.setTimeMin(time);
		mDecoder.setTimeMin(time);
	}

	public void setTimeMax(long time) {
		mCounterAccel.setOverTime(time);
		mDecoder.setTimeMax(time);
	}
}