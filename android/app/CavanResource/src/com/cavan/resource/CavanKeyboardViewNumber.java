package com.cavan.resource;

import android.content.Context;
import android.util.AttributeSet;

import com.cavan.android.CavanKeyboardView;

public class CavanKeyboardViewNumber extends CavanKeyboardView {

	public static final int KEYBOARD_RADIX10 = 0;
	public static final int KEYBOARD_RADIX16 = 1;
	public static final int KEYBOARD_RADIX8 = 2;
	public static final int KEYBOARD_RADIX2 = 3;

	public static final int[] RESOURCES = {
		R.xml.keyboard_number_radix10,
		R.xml.keyboard_number_radix16,
		R.xml.keyboard_number_radix8,
		R.xml.keyboard_number_radix2,
	};

	public CavanKeyboardViewNumber(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
	}

	public CavanKeyboardViewNumber(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	public CavanKeyboardViewNumber(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	@Override
	protected int[] getKeyboardResources() {
		return RESOURCES;
	}
}
