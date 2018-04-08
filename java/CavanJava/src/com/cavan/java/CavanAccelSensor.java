package com.cavan.java;

public abstract class CavanAccelSensor {

	private double mAxisX;
	private double mAxisY;
	private double mAxisZ;

	public abstract void putBytes(byte[] bytes);

	public CavanAccelSensor() {
		super();
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

	public double getAxisX2() {
		return 0;
	}

	public double getAxisY2() {
		return 0;
	}

	public double getAxisZ2() {
		return 0;
	}

	public String getAccelText() {
		return String.format("[%5.2f, %5.2f, %5.2f]", mAxisX, mAxisY, mAxisZ);
	}
}
