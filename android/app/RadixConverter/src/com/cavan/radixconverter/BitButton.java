package com.cavan.radixconverter;

import android.content.Context;
import android.graphics.Color;
import android.widget.Button;

public class BitButton extends Button {

	private BitAdapter mAdapter;

	public BitButton(Context context, BitAdapter adapter) {
		super(context);

		mAdapter = adapter;
	}

	public BitAdapter getAdapter() {
		return mAdapter;
	}

	public void setActive(boolean active) {
		setTextColor(active ? Color.RED : Color.WHITE);
	}
}
