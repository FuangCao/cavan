package com.cavan.java;

public class CavanIndexGenerator {

	private int mIndex;

	public int genIndex() {
		if (++mIndex < 0) {
			mIndex = 0;
		}

		return mIndex;
	}

	public void setIndex(int index) {
		mIndex = index;
	}
}
