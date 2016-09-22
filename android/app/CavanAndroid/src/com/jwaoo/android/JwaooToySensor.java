package com.jwaoo.android;

import com.cavan.java.CavanAccelSensor;

public abstract class JwaooToySensor extends CavanAccelSensor {

	public static final int SENSOR_COUNT = 4;
	public static final double CAPACITY_MAX = 128;
	public static final double CAPACITY_MIN = -128;

	private double[] mCapacitys = new double[SENSOR_COUNT];

	public abstract int getCapacityValueCount();

	public JwaooToySensor() {
		super();
	}

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

	public String getCapacityText() {
		return String.format("[%7.2f, %7.2f, %7.2f, %7.2f]", mCapacitys[0], mCapacitys[1], mCapacitys[2], mCapacitys[3]);
	}

	public void setCapacity(int index, double capacity) {
		mCapacitys[index] = capacity;
	}
}
