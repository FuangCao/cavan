package com.cavan.java;

public class Mpu6050Parser extends CavanByteCache {

	private float mAccelX;
	private float mAccelY;
	private float mAccelZ;
	private float mGyroX;
	private float mGyroY;
	private float mGyroZ;
	private float mTempture;

	public Mpu6050Parser(byte[] bytes) {
		super(bytes);

		mAccelX = readAccelValue();
		mAccelY = readAccelValue();
		mAccelZ = readAccelValue();
		mTempture = readTemptureValue();
		mGyroX = readGyroValue();
		mGyroY = readGyroValue();
		mGyroZ = readGyroValue();
	}

	public float getAccelX() {
		return mAccelX;
	}

	public float getAccelY() {
		return mAccelY;
	}

	public float getAccelZ() {
		return mAccelZ;
	}

	public float getAccelJoin() {
		return (float) Math.sqrt(mAccelX * mAccelX + mAccelY + mAccelY + mAccelZ * mAccelZ);
	}

	public float getGyroX() {
		return mGyroX;
	}

	public float getGyroY() {
		return mGyroY;
	}

	public float getGyroZ() {
		return mGyroZ;
	}

	public float getTempture() {
		return mTempture;
	}

	public float readCoord() {
		return readValueBe16();
	}

	public float readAccelValue() {
		return (float) (readCoord() * 9.8 / 16384);
	}

	public float readGyroValue() {
		return readCoord() / 131;
	}

	public float readTemptureValue() {
		return (readCoord() + 13200) / 280 - 13;
	}
}
