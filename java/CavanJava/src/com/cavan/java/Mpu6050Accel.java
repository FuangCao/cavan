package com.cavan.java;

public class Mpu6050Accel extends AccelDataCache {

	public Mpu6050Accel(byte[] bytes) {
		super(bytes);
	}

	@Override
	protected double readAccelValue() {
		return readValueBe16() * 9.8 / 16384;
	}
}