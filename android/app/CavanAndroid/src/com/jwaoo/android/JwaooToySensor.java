package com.jwaoo.android;

import com.cavan.java.CavanAccelSensor;

public abstract class JwaooToySensor extends CavanAccelSensor {

	public static final int SENSOR_COUNT = 3;
	public static final int DEPTH_MAX = 127;

	private int[] mCapacitys = new int[SENSOR_COUNT];

	public int[] getCapacitys() {
		return mCapacitys;
	}

	public int getCapacity(int index) {
		return mCapacitys[index];
	}

	public int getCapacitySum() {
		int capacity = 0;

		for (int i = mCapacitys.length - 1; i >= 0; i--) {
			capacity += mCapacitys[i];
		}

		return capacity;
	}

	public  int getDepth() {
		int capacity = 0;

		for (int i = mCapacitys.length - 1; i >= 0; i--) {
			if (mCapacitys[i] > 0) {
				capacity += mCapacitys[i];
			}
		}

		return capacity / SENSOR_COUNT;
	}

	public void setCapacity(int index, int capacity) {
		mCapacitys[index] = capacity;
	}
}
