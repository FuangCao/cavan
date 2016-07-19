package com.jwaoo.android;

import com.cavan.java.CavanByteCache;

public class JwaooToySensorMpu6050 extends JwaooToySensor {

	private CavanByteCache mCache = new CavanByteCache();

	private double readAxis() {
		return mCache.readValue8() * 9.8 / 64;
	}

	@Override
	public void putBytes(byte[] bytes) {
		mCache.setBytes(bytes);
		setAxisX(readAxis());
		setAxisY(readAxis());
		setAxisZ(readAxis());
		setCapacity(0, mCache.readValue8() & 0xFF);
		setCapacity(1, mCache.readValue8() & 0xFF);
		setCapacity(2, mCache.readValue8() & 0xFF);
		setCapacity(3, mCache.readValue8() & 0xFF);
		updateDepth();
	}
}
