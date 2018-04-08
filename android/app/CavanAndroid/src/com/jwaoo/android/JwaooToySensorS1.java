package com.jwaoo.android;

import com.cavan.java.CavanJava;

public class JwaooToySensorS1 extends JwaooToySensor {

	private double mAxisX2;
	private double mAxisY2;
	private double mAxisZ2;

	@Override
	public double getAxisX2() {
		return mAxisX2;
	}

	public void setAxisX2(double x) {
		mAxisX2 = x;
	}

	@Override
	public double getAxisY2() {
		return mAxisY2;
	}

	public void setAxisY2(double y) {
		mAxisY2 = y;
	}

	@Override
	public double getAxisZ2() {
		return mAxisZ2;
	}

	public void setAxisZ2(double z) {
		mAxisZ2 = z;
	}

	private double readAxis1(byte[] bytes, int offset) {
		return CavanJava.buildValue16(bytes, offset) * 9.8 / 16384;
	}

	private double readAxis2(byte[] bytes, int offset) {
		return bytes[offset] * 9.8 / 64;
	}

	@Override
	public void putBytes(byte[] bytes) {
		setAxisY(readAxis1(bytes, 0));
		setAxisX(readAxis1(bytes, 2));
		setAxisZ(readAxis1(bytes, 4));

		if (bytes.length >= 9) {
			mAxisY2 = -readAxis2(bytes, 6);
			mAxisX2 = -readAxis2(bytes, 7);
			mAxisZ2 = readAxis2(bytes, 8);
		}
	}

	@Override
	public int getCapacityValueCount() {
		return 0;
	}

	@Override
	public String getAccelText() {
		return super.getAccelText() + String.format(" [%5.2f, %5.2f, %5.2f]", mAxisX2, mAxisY2, mAxisZ2);
	}
}