package com.cavan.jwaootoyfactorytest;

import android.content.Context;
import android.graphics.Color;
import android.util.AttributeSet;
import android.widget.TextView;

public class JwaooSensorTestView extends TextView {

	private boolean mPassed;
	private double mValueMin;
	private double mValueMax;
	private double mDifferenceMin;

	public JwaooSensorTestView(Context context) {
		super(context);
		initValues();
	}

	public JwaooSensorTestView(Context context, AttributeSet attrs) {
		super(context, attrs);
		initValues();
	}

	public JwaooSensorTestView(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
		initValues();
	}

	public JwaooSensorTestView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
		initValues();
	}

	public void setDifferenceMin(double min) {
		mDifferenceMin = min;
		initValues();
	}

	public double getDifference() {
		return mValueMax - mValueMin;
	}

	public boolean isPassed() {
		return mPassed;
	}

	public void initValues() {
		mPassed = false;
		mValueMin = Double.POSITIVE_INFINITY;
		mValueMax = Double.NEGATIVE_INFINITY;
	}

	public boolean putValueValue(double value) {
		setText(String.format("%4.2f", value));

		if (value > mValueMax) {
			mValueMax = value;
		} else if (value < mValueMin) {
			mValueMin = value;
		}

		int color;

		if (mPassed || getDifference() > mDifferenceMin) {
			mPassed = true;
			color = Color.GREEN;
		} else {
			color = Color.GRAY;
		}

		setBackgroundColor(color);

		return mPassed;
	}
}
