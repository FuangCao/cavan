package com.cavan.android;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.CheckBox;

public class CavanCheckBox extends CheckBox {

	private boolean mCheckedChangeEventEnable;
	private OnCheckedChangeListener mOnCheckedChangeListener;

	public CavanCheckBox(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
	}

	public CavanCheckBox(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	public CavanCheckBox(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public CavanCheckBox(Context context) {
		super(context);
	}

	public boolean isCheckedChangeEventEnabled() {
		return mCheckedChangeEventEnable;
	}

	public void setCheckedChangeEventEnable(boolean enable) {
		mCheckedChangeEventEnable = enable;

		if (enable) {
			super.setOnCheckedChangeListener(mOnCheckedChangeListener);
		} else {
			super.setOnCheckedChangeListener(null);
		}
	}

	public void setCheckedSilent(boolean checked) {
		super.setOnCheckedChangeListener(null);
		setChecked(checked);
		super.setOnCheckedChangeListener(mOnCheckedChangeListener);
	}

	@Override
	public void setOnCheckedChangeListener(OnCheckedChangeListener listener) {
		super.setOnCheckedChangeListener(listener);
		mOnCheckedChangeListener = listener;
		mCheckedChangeEventEnable = true;
	}
}
