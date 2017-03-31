package com.cavan.resource;

import android.content.Context;
import android.util.AttributeSet;

import com.cavan.android.CavanKeyboardView;

public class CavanKeyboardViewHex extends CavanKeyboardView {

	public CavanKeyboardViewHex(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
	}

	public CavanKeyboardViewHex(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	public CavanKeyboardViewHex(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	@Override
	protected int getKeyboardResource() {
		return R.xml.keyboard_hex;
	}
}
