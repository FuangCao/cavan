package com.cavan.java;

public abstract class CavanAccelSensor {

	double mAxisX;
	double mAxisY;
	double mAxisZ;

	public abstract void putBytes(byte[] bytes);

	public CavanAccelSensor() {
		mAxisX = mAxisY = mAxisZ = 0;
	}

	public CavanAccelSensor(byte[] bytes) {
		putBytes(bytes);
	}

	public double getAxisX() {
		return mAxisX;
	}

	public void setAxisX(double x) {
		mAxisX = x;
	}

	public double getAxisY() {
		return mAxisY;
	}

	public void setAxisY(double y) {
		mAxisY = y;
	}

	public double getAxisZ() {
		return mAxisZ;
	}

	public void setAxisZ(double z) {
		mAxisZ = z;
	}

	public String getAccelText() {
		return String.format("[%5.2f, %5.2f, %5.2f]", mAxisX, mAxisY, mAxisZ);
	}
}
