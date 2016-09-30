package com.cavan.jwaootoyfactorytest;

import android.content.Context;
import android.graphics.Color;
import android.util.AttributeSet;
import android.widget.TextView;

public class JwaooSensorTestView extends TextView {

	private boolean mPassed;
	private double mDifferenceMin;
	private double mValueMin = Double.MAX_VALUE;
	private double mValueMax = Double.MIN_VALUE;

	public JwaooSensorTestView(Context context) {
		super(context);
	}

	public JwaooSensorTestView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public JwaooSensorTestView(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	public JwaooSensorTestView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
	}

	public void setDifferenceMin(double min) {
		mDifferenceMin = min;
	}

	public double getDifference() {
		return mValueMax - mValueMin;
	}

	public boolean isPassed() {
		return mPassed;
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
