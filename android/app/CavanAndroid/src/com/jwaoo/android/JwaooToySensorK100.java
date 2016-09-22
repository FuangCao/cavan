package com.jwaoo.android;

import com.cavan.java.CavanByteCache;

public class JwaooToySensorK100 extends JwaooToySensor {

	private CavanByteCache mCache = new CavanByteCache();

	private double readAxis() {
		return mCache.readValue8() * 9.8 / 64;
	}

	private double readCapacity8() {
		return mCache.readValue8();
	}

	private double readCapacity16() {
		return ((double) mCache.readValueBe16()) / 256;
	}

	@Override
	public void putBytes(byte[] bytes) {
		mCache.setBytes(bytes);
		setAxisX(readAxis());
		setAxisY(readAxis());
		setAxisZ(readAxis());

		if (bytes.length < 11) {
			setCapacity(2, readCapacity8());
			setCapacity(1, readCapacity8());
			setCapacity(0, readCapacity8());
		} else {
			setCapacity(0, readCapacity16());
			setCapacity(1, readCapacity16());
			setCapacity(2, readCapacity16());
			setCapacity(3, readCapacity16());
		}
	}

	@Override
	public int getCapacityValueCount() {
		return 4;
	}
}
