package com.jwaoo.android;

import com.cavan.java.CavanAccelFreqParser;

public class JwaooToyParser extends CavanAccelFreqParser {

	private int mDepth;

	public JwaooToyParser(long timeFuzz, double valueFuzz) {
		super(timeFuzz, valueFuzz);
	}

	protected void onDepthChanged(int depth) {}

	private void updateDepth(int depth) {
		if (mDepth != depth) {
			mDepth = depth;
			onDepthChanged(depth);
		}
	}

	public int getDepth() {
		return mDepth;
	}

	public void putData(JwaooToySensor sensor) {
		super.putData(sensor);
		updateDepth(sensor.getDepth());
	}
}
