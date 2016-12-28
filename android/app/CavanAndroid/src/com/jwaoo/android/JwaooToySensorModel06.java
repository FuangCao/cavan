package com.jwaoo.android;

import com.cavan.java.CavanJava;

public class JwaooToySensorModel06 extends JwaooToySensor {

	private double readAxis(byte[] bytes, int offset) {
		return CavanJava.buildValue16(bytes, offset) * 9.8 / 16384;
	}

	@Override
	public void putBytes(byte[] bytes) {
		setAxisX(readAxis(bytes, 0));
		setAxisY(readAxis(bytes, 2));
		setAxisZ(readAxis(bytes, 4));
	}

	@Override
	public int getCapacityValueCount() {
		return 0;
	}
}