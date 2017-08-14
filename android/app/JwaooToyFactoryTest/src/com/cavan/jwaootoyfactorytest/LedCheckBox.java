package com.cavan.jwaootoyfactorytest;

import com.jwaoo.android.JwaooBleToy;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.widget.CheckBox;

public class LedCheckBox extends CheckBox {

	private int mLedIndex;
	private int mCheckCount;
	private JwaooBleToy mBleToy;

	public LedCheckBox(Context context) {
		super(context);
	}

	public LedCheckBox(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public LedCheckBox(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	public LedCheckBox(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
	}

	public void init(JwaooBleToy bleToy, int index) {
		mBleToy = bleToy;
		mLedIndex = index;
		mCheckCount = 0;
	}

	public int getLedIndex() {
		return mLedIndex;
	}

	public void setLedIndex(int index) {
		mLedIndex = index;
	}

	public JwaooBleToy getBleToy() {
		return mBleToy;
	}

	public void setBleToy(JwaooBleToy bleToy) {
		mBleToy = bleToy;
	}

	public int getCheckCount() {
		return mCheckCount;
	}

	public void setCheckCount(int checkCount) {
		mCheckCount = checkCount;
	}

	public int addCheckCount() {
		return ++mCheckCount;
	}

	public boolean isPassable() {
		if (getVisibility() != View.VISIBLE) {
			return true;
		}

		return mCheckCount > 1;
	}

	public boolean setLedEnable(boolean enable) {
		try {
			if (mBleToy.setLedEnable(mLedIndex, enable)) {
				mCheckCount++;
				return true;
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

		return false;
	}
}
