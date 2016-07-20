package com.jwaoo.android;

import com.cavan.java.CavanAccelSensor;

public abstract class JwaooToySensor extends CavanAccelSensor {

	public static final int MAX_DEPTH = 127;

	private double mDepth;
	private int[] mCapacitys = new int[4];

	public int[] getCapacitys() {
		return mCapacitys;
	}

	public int getCapacity(int index) {
		return mCapacitys[index];
	}

	public int getDepth() {
		return (int) mDepth;
	}

	protected void updateDepth() {
		double depth = 0;

		// CavanAndroid.logE(String.format("capacity: [%4d, %4d, %4d, %4d]", mCapacitys[0], mCapacitys[1], mCapacitys[2], mCapacitys[3]));

		for (int capacity : mCapacitys) {
			if (capacity > 0) {
				depth += capacity;
			}
		}

		mDepth = (mDepth + (depth / 4)) / 2;
	}

	public void setCapacity(int index, int capacity) {
		mCapacitys[index] = capacity;
	}
}
