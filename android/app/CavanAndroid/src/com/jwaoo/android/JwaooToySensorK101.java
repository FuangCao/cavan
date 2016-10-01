package com.jwaoo.android;

public class JwaooToySensorK101 extends JwaooToySensor {

	private double readAxis(byte value) {
		return value * 9.8 / 64;
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
		setAxisX(readAxis(bytes[0]));
		setAxisY(readAxis(bytes[1]));
		setAxisZ(readAxis(bytes[2]));
		setCapacity(bytes[3]);
	}

	@Override
	public int getCapacityValueCount() {
		return 3;
	}
}
