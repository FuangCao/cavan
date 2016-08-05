package com.jwaoo.android;

import com.cavan.java.CavanSquareWaveCounter;

public class JwaooToyParser {

	private double mFreq;
	private double mDepth;

	private CavanSquareWaveCounter mCounterAccel;
	private CavanSquareWaveCounter mCounterDepth;
	private JwaooDepthDecoder mDecoder;

	public JwaooToyParser(double accelFuzz, double depthFuzz, long timeMin, long timeMax) {
		mCounterAccel = new CavanSquareWaveCounter(accelFuzz, timeMin, timeMax);
		mCounterDepth = new CavanSquareWaveCounter(depthFuzz, timeMin, timeMax);
		mDecoder = new JwaooDepthDecoder(depthFuzz, timeMin, timeMax);
	}

	public double getFreq() {
		return mFreq;
	}

	public int getDepth() {
		return (int) (mDepth * JwaooToySensor.DEPTH_MAX);
	}

	public void putData(JwaooToySensor sensor) {
		mCounterAccel.putFreqValue(sensor.getAxisX());
		mCounterDepth.putFreqValue(sensor.getCapacitySum());

		if (mCounterAccel.getValueRangeAvg() > 2.0) {
			mFreq = mCounterAccel.getFreq();
		} else {
			mFreq = mCounterDepth.getFreq();
		}

		mDepth = mDecoder.putValue(sensor.getCapacitys());
	}

	public void setAccelFuzz(double fuzz) {
		mCounterAccel.setValueFuzz(fuzz);
	}

	public void setDepthFuzz(double fuzz) {
		mCounterDepth.setValueFuzz(fuzz);
		mDecoder.setValueFuzz(fuzz);
	}

	public void setTimeMin(long time) {
		mCounterAccel.setTimeMin(time);
		mCounterDepth.setTimeMin(time);
		mDecoder.setTimeMin(time);
	}

	public void setTimeMax(long time) {
		mCounterAccel.setTimeMax(time);
		mCounterDepth.setTimeMax(time);
		mDecoder.setTimeMax(time);
	}
}