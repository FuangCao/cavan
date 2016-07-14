package com.cavan.java;

public class CavanSensorMpu6050 extends CavanAccelSensor {

	private CavanByteCache mCache = new CavanByteCache();

	public CavanSensorMpu6050() {
		super();
	}

	public CavanSensorMpu6050(byte[] bytes) {
		super(bytes);
	}

	public double readAxis() {
		return mCache.readValue8() * 9.8 / 64;
	}

	@Override
	public void putBytes(byte[] bytes) {
		mCache.setBytes(bytes);

		setAxisX(readAxis());
		setAxisY(readAxis());
		setAxisZ(readAxis());
	}
}