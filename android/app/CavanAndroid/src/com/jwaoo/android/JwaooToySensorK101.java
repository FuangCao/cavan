package com.jwaoo.android;

import com.cavan.java.CavanJava;

public class JwaooToySensorK101 extends JwaooToySensor {

	private double readAxis(byte value) {
		return value * 9.8 / 64;
	}

	private double readAxis(byte[] bytes, int offset) {
		return CavanJava.buildValue16(bytes, offset) * 9.8 / 16384;
	}

	private double readCapacity(byte value, int index) {
		if ((value & (1 << index)) == 0) {
			return 0;
		}

		return CAPACITY_MAX;
	}

	private void setCapacity(byte value) {
		setCapacity(0, readCapacity(value, 2));
		setCapacity(1, readCapacity(value, 1));
		setCapacity(2, readCapacity(value, 0));
	}

	@Override
	public void putBytes(byte[] bytes) {
		if (bytes.length == 6) {
			setAxisX(readAxis(bytes, 0));
			setAxisY(readAxis(bytes, 2));
			setAxisZ(readAxis(bytes, 4));
		} else {
			setAxisX(readAxis(bytes[0]));
			setAxisY(readAxis(bytes[1]));
			setAxisZ(readAxis(bytes[2]));
			setCapacity(bytes[3]);
		}
	}

	@Override
	public int getCapacityValueCount() {
		return 3;
	}
}
