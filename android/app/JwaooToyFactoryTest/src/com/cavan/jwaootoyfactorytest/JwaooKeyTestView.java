package com.cavan.jwaootoyfactorytest;

import android.content.Context;
import android.graphics.Color;
import android.util.AttributeSet;
import android.widget.Button;

public class JwaooKeyTestView extends Button {

	private String mTextRaw;
	private int mPressCount;
	private boolean mPressed;

	public JwaooKeyTestView(Context context) {
		super(context);
	}

	public JwaooKeyTestView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public JwaooKeyTestView(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	public JwaooKeyTestView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
	}

	public void updateText() {
		int bColor, tColor;
		String text = mTextRaw;

		if (mPressCount > 0) {
			bColor =  Color.GREEN;

			if (text != null) {
				text += ": " + mPressCount;
			}
		} else {
			bColor = Color.GRAY;
		}

		if (mPressed) {
			tColor = Color.RED;
		} else {
			tColor = Color.BLACK;
		}

		super.setText(text);
		super.setTextColor(tColor);
		super.setBackgroundColor(bColor);
	}

	public int getPressCount() {
		return mPressCount;
	}

	public void setPressCount(int count) {
		mPressCount = count;
		updateText();
	}

	public void setTextRaw(String text) {
		mTextRaw = text;
		updateText();
	}

	public void setTextRaw(int resId) {
		setTextRaw(getResources().getString(resId));
	}

	public boolean getPressState() {
		return mPressed;
	}

	public void setPressState(boolean pressed) {
		mPressed = pressed;

		if (pressed) {
			mPressCount++;
		}

		updateText();
	}

	public boolean isTestPass() {
		if (mPressed) {
			return mPressCount > 1;
		}

		return mPressCount > 0;
	}

	public boolean isTestFail() {
		if (mPressed) {
			return mPressCount < 2;
		}

		return mPressCount < 1;
	}
}
