package com.cavan.java;

public class CavanIndexGenerator {

	private int mIndex;

	synchronized public int genIndex() {
		if (++mIndex < 0) {
			mIndex = 0;
		}

		return mIndex;
	}

	synchronized public void setIndex(int index) {
		mIndex = index;
	}
}
