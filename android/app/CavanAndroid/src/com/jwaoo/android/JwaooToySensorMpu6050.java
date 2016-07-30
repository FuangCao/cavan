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

		if (bytes.length < 11) {
			setCapacity(0, mCache.readValue8());
			setCapacity(1, mCache.readValue8());
			setCapacity(2, mCache.readValue8());
			setCapacity(3, mCache.readValue8());
		} else {
			setCapacity(0, mCache.readValueBe16());
			setCapacity(1, mCache.readValueBe16());
			setCapacity(2, mCache.readValueBe16());
			setCapacity(3, mCache.readValueBe16());
		}

		updateDepth();
	}
}
