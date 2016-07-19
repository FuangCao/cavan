package com.jwaoo.android;

import com.cavan.java.CavanAccelSensor;

public abstract class JwaooToySensor extends CavanAccelSensor {

	public static final int MAX_DEPTH = 100;

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

		for (int capacity : mCapacitys) {
			depth += capacity;
		}

		depth = depth * MAX_DEPTH / 1020;
		mDepth = (mDepth + depth) / 2;
	}

	public void setCapacity(int index, int capacity) {
		mCapacitys[index] = capacity;
	}
}