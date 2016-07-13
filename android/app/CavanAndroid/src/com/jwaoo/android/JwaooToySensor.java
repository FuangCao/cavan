package com.jwaoo.android;

import com.cavan.java.AccelFreqParser;
import com.cavan.java.Mpu6050Accel;

public class JwaooToySensor extends AccelFreqParser {

	private int mDepth;

	public JwaooToySensor(long timeFuzz, double valueFuzz) {
		super(timeFuzz, valueFuzz);
	}

	protected void onDepthChanged(int depth) {

	}

	private void updateDepth(int depth) {
		if (mDepth != depth) {
			mDepth = depth;
			onDepthChanged(depth);
		}
	}

	public int getDepth() {
		return mDepth;
	}

	public void putData(byte[] bytes) {
		Mpu6050Accel accel = new Mpu6050Accel(bytes);
		updateDepth(accel.readValue8());
		putValue(accel);
	}
}
