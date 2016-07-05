package com.cavan.java;

public abstract class AccelDataCache extends CavanByteCache {

	private double mCoorX;
	private double mCoorY;
	private double mCoorZ;

	protected abstract double readAccelValue();

	public AccelDataCache(byte[] bytes, int offset, int length) {
		super(bytes, offset, length);

		mCoorX = readAccelValue();
		mCoorY = readAccelValue();
		mCoorZ = readAccelValue();
	}

	public AccelDataCache(byte[] bytes, int length) {
		this(bytes, 0, length);
	}

	public AccelDataCache(byte[] bytes) {
		this(bytes, bytes.length);
	}

	public double getCoorX() {
		return mCoorX;
	}

	public double getCoorY() {
		return mCoorY;
	}

	public double getCoorZ() {
		return mCoorZ;
	}
}
