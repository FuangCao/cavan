package com.cavan.android;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.EditText;

public class CavanEditText extends EditText {

	public interface OnSelectionChangedListener {

		void onSelectionChanged(int start, int end);
	}

	private OnSelectionChangedListener mOnSelectionChangedListener;

	public CavanEditText(Context context) {
		super(context);
	}

	public CavanEditText(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public CavanEditText(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	public CavanEditText(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
	}

	public OnSelectionChangedListener getOnSelectionChangedListener() {
		return mOnSelectionChangedListener;
	}

	public void setOnSelectionChangedListener(OnSelectionChangedListener listener) {
		mOnSelectionChangedListener = listener;
	}

	@Override
	protected void onSelectionChanged(int selStart, int selEnd) {
		super.onSelectionChanged(selStart, selEnd);

		if (mOnSelectionChangedListener != null) {
			mOnSelectionChangedListener.onSelectionChanged(selStart, selEnd);
		}
	}
}
