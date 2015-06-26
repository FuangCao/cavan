package com.cavan.radixconverter;

import android.content.Context;
import android.graphics.Color;
import android.widget.Button;

public class BitButton extends Button {

	private int mIndex;
	private BitAdapter mAdapter;

	public BitButton(Context context, BitAdapter adapter, int index) {
		super(context);

		mAdapter = adapter;
		mIndex = index;
	}

	public BitAdapter getAdapter() {
		return mAdapter;
	}

	public int getIndex() {
		return mIndex;
	}

	public void setActive(boolean active) {
		setTextColor(active ? Color.RED : Color.WHITE);
	}

	public BitButton getNextButton() {
		if (mIndex > 0) {
			return mAdapter.getButton(mIndex - 1);
		} else {
			return null;
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

	public int getValue() {
		return parseText(getText());
	}

	public void setValue(int value) {
		setText(KeypadAdapter.VALUE_TEXT_MAP[value]);
	}

	public void add(int value) {
		setValue((getValue() + value) % mAdapter.getBase());
	}
}
