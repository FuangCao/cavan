package com.jwaoo.android;

import com.cavan.java.CavanJava;

public class JwaooToySensorS1 extends JwaooToySensor {

	private double readAxis1(byte[] bytes, int offset) {
		return CavanJava.buildValue16(bytes, offset) * 9.8 / 16384;
	}

	private double readAxis2(byte[] bytes, int offset) {
		return (bytes[offset] & 0xFF) * 9.8 / 64;
	}

	@Override
	public void putBytes(byte[] bytes) {
		setAxisY(readAxis1(bytes, 0));
		setAxisX(readAxis1(bytes, 2));
		setAxisZ(readAxis1(bytes, 4));
		setAxisY(readAxis2(bytes, 6));
		setAxisX(readAxis2(bytes, 7));
		setAxisZ(readAxis2(bytes, 8));
	}

	@Override
	public int getCapacityValueCount() {
		return 0;
	}
}