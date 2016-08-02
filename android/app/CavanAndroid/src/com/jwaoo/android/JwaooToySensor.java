package com.jwaoo.android;

import com.cavan.java.CavanAccelSensor;

public abstract class JwaooToySensor extends CavanAccelSensor {

	public static final int SENSOR_COUNT = 3;
	public static final double DEPTH_MAX = 100;
	public static final double CAPACITY_MAX = 127;
	public static final double CAPACITY_MIN = -128;

	private double[] mCapacitys = new double[4];

	public double[] getCapacitys() {
		return mCapacitys;
	}

	public double getCapacity(int index) {
		return mCapacitys[index];
	}

	public double getCapacitySum() {
		double capacity = 0;

		for (int i = mCapacitys.length - 1; i >= 0; i--) {
			capacity += mCapacitys[i];
		}

		return capacity;
	}

	public  double getDepth() {
		double capacity = 0;

		for (int i = mCapacitys.length - 1; i >= 0; i--) {
			if (mCapacitys[i] > 0) {
				capacity += mCapacitys[i];
			}
		}

		return capacity / SENSOR_COUNT;
	}

	public void setCapacity(int index, double capacity) {
		mCapacitys[index] = capacity;
	}
}
