package com.cavan.java;

public abstract class CavanAccelSensor {

	private double mAxisX;
	private double mAxisY;
	private double mAxisZ;
	private double mAxisX2;
	private double mAxisY2;
	private double mAxisZ2;

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
		return mAxisX2;
	}

	public void setAxisX2(double x) {
		mAxisX2 = x;
	}

	public double getAxisY2() {
		return mAxisY2;
	}

	public void setAxisY2(double y) {
		mAxisY2 = y;
	}

	public double getAxisZ2() {
		return mAxisZ2;
	}

	public void setAxisZ2(double z) {
		mAxisZ2 = z;
	}

	public String getAccelText() {
		return String.format("[%5.2f, %5.2f, %5.2f]", mAxisX, mAxisY, mAxisZ);
	}
}
