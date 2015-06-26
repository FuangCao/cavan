package com.cavan.radixconverter;

import android.content.Context;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;

public class BitAdapter extends BaseAdapter {

	private Context mContext;
	private OnClickListener mListener;

	private int mCount;
	private int mBase;
	private BitButton mButtons[];

	public BitAdapter(Context context, OnClickListener listener, int count, int base) {
		super();

		mContext = context;
		mListener = listener;

		mCount = count;
		mBase = base;
		mButtons = new BitButton[mCount];
	}

	@Override
	public int getCount() {
		return mCount;
	}

	@Override
	public Object getItem(int position) {
		return null;
	}

	@Override
	public long getItemId(int position) {
		return 0;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		int offset = mCount - position - 1;

		BitButton button = mButtons[offset];
		if (button == null) {
			mButtons[offset] = button = new BitButton(mContext, this);
			button.setText("0");
			button.setOnClickListener(mListener);
		}

		return button;
	}

	public int getBase() {
		return mBase;
	}

	public void setValue(long value) {
		for (int i = 0; i < mCount; i++, value /= mBase) {
			if (mButtons[i] != null) {
				mButtons[i].setText(KeypadAdapter.VALUE_TEXT_MAP[(int) (value % mBase)]);
			}
		}
	}

	public int parseText(CharSequence charSequence) {
		char c = charSequence.charAt(0);
		if (c >= '0' && c <= '9') {
			return c - '0';
		} else if (c >= 'A' && c <= 'F') {
			return c - 'A' + 10;
		} else if (c >= 'a' && c <= 'f') {
			return c - 'a' + 10;
		}

		return -1;
	}

	public long getValue() {
		long value = 0;

		for (int i = mCount - 1; i >= 0; i--) {
			value = value * mBase + parseText(mButtons[i].getText());
		}

		return value;
	}
}
