package com.jwaoo.android;

import com.cavan.java.CavanJava;

public class JwaooToySensorModel10 extends JwaooToySensor {

	private double readAxis(byte[] bytes, int offset) {
		return CavanJava.buildValue16(bytes, offset) * 9.8 / 16384;
	}

	@Override
	public void putBytes(byte[] bytes) {
		setAxisY(readAxis(bytes, 0));
		setAxisX(readAxis(bytes, 2));
		setAxisZ(readAxis(bytes, 4));
	}

	@Override
	public int getCapacityValueCount() {
		return 0;
	}
}