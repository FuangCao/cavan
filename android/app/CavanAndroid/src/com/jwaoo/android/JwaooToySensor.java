package com.jwaoo.android;

import com.cavan.java.CavanAccelSensor;

public abstract class JwaooToySensor extends CavanAccelSensor {

	private int mDepth;

	public JwaooToySensor() {
		mDepth = 0;
	}

	public int getDepth() {
		return mDepth;
	}

	public void setDepth(int depth) {
		mDepth = depth;
	}
}